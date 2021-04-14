#include "aio_read_util.h"

#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "coroutine.h"
#include "defines.h"
#include "sort.h"

struct request {
  int32_t status;
  struct aiocb *aiocb_p;
};

uint32_t get_filesize(int fd) {
  struct stat st;
  conditional_handle_error(fstat(fd, &st) == -1, "fstat error");

  return st.st_size;
}

void aio_read_util(uint32_t num_files, char **filenames, char **buffers,
                   int32_t **arrays, int32_t *sizes) {
  struct request *req_list = calloc(num_files, sizeof(struct request));
  conditional_handle_error(req_list == NULL,
                           "calloc request list error");

  struct aiocb *aiocb_list = calloc(num_files, sizeof(struct aiocb));
  conditional_handle_error(aiocb_list == NULL,
                           "calloc aiocb list error");

  int *processed_mask = calloc(num_files, sizeof(int));
  conditional_handle_error(processed_mask == NULL,
                           "calloc processed mask error");

  for (uint32_t j = 0; j < num_files; ++j) {
    req_list[j].status = EINPROGRESS;
    req_list[j].aiocb_p = &aiocb_list[j];

    int fd = open(filenames[j], O_RDONLY);
    conditional_handle_error(fd == -1, "error while opening file");

    req_list[j].aiocb_p->aio_fildes = fd;

    uint32_t buf_size = get_filesize(req_list[j].aiocb_p->aio_fildes) + 1;
    req_list[j].aiocb_p->aio_buf = malloc(buf_size);
    conditional_handle_error(req_list[j].aiocb_p->aio_buf == NULL,
                             "malloc aio buffer error");

    req_list[j].aiocb_p->aio_nbytes = buf_size;

    req_list[j].aiocb_p->aio_reqprio = 0;

    req_list[j].aiocb_p->aio_offset = 0;

    conditional_handle_error(aio_read(req_list[j].aiocb_p) == -1,
                             "aio_read error");
  }

  uint32_t num_finished = 0;
  while (num_finished != num_files) {
    for (uint32_t i = 0; i < num_files; ++i) {
      req_list[i].status = aio_error(req_list[i].aiocb_p);
      if (req_list[i].status != EINPROGRESS && processed_mask[i] == 0) {
        ++num_finished;
        processed_mask[i] = 1;

        if (req_list[i].status != 0) {
          perror("aio_error");
        }

        char *line = (char *)req_list[i].aiocb_p->aio_buf;
        uint32_t line_length = req_list[i].aiocb_p->aio_nbytes;
        line[line_length - 1] = '\0';

        // spawning coroutine to sort file
        struct ucontext_t *ctx = spawn_coroutine(&global_coro_scheduler);
        makecontext(ctx, (void (*)(void))prepare_sort, 4, line, line_length,
                    &arrays[i], &sizes[i]);
      }
    }

    // yielding to let other coroutines work
    // instead of waiting for async reading to finish
    yield(&global_coro_scheduler);
  }

  for (uint32_t j = 0; j < num_files; ++j) {
    close(req_list[j].aiocb_p->aio_fildes);
  }

  // to free later
  for (uint32_t i = 0; i < num_files; ++i) {
    buffers[i] = (char *)req_list[i].aiocb_p->aio_buf;
  }

  free(processed_mask);
  free(aiocb_list);
  free(req_list);
}

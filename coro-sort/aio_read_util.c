#include "aio_read_util.h"

#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "coro_spawn_to_sort.h"
#include "defines.h"

#define IO_SIGNAL SIGUSR1

struct request {
  int32_t status;
  struct aiocb *aiocb_p;
};

static void aio_finish_handler(int sig, siginfo_t *si, void *ucontext) {
  if (si->si_code == SI_ASYNCIO) {
    int *flag_p = (int *)si->si_value.sival_ptr;
    *flag_p = 1;
  }
}

uint32_t get_filesize(int fd) {
  struct stat st;
  fstat(fd, &st);

  return st.st_size;
}

void aio_read_util(uint32_t num_files, char **filenames) {
  struct request *req_list = calloc(num_files, sizeof(struct request));
  struct aiocb *aiocb_list = calloc(num_files, sizeof(struct aiocb));
  int *fin_mask = calloc(num_files, sizeof(int));
  int *processed_mask = calloc(num_files, sizeof(int));

  struct sigaction sig_act;
  sigemptyset(&sig_act.sa_mask);
  sig_act.sa_flags = SA_RESTART | SA_SIGINFO;
  sig_act.sa_sigaction = aio_finish_handler;
  conditional_handle_error(sigaction(IO_SIGNAL, &sig_act, NULL) == -1,
                           "sigaction error");

  for (uint32_t j = 0; j < num_files; ++j) {
    req_list[j].status = EINPROGRESS;
    req_list[j].aiocb_p = &aiocb_list[j];

    int fd = open(filenames[j], O_RDONLY);
    conditional_handle_error(fd == -1, "error while opening file");

    req_list[j].aiocb_p->aio_fildes = fd;

    uint32_t buf_size = get_filesize(req_list[j].aiocb_p->aio_fildes) + 1;
    req_list[j].aiocb_p->aio_buf = malloc(buf_size);
    req_list[j].aiocb_p->aio_nbytes = buf_size;

    req_list[j].aiocb_p->aio_reqprio = 0;

    req_list[j].aiocb_p->aio_offset = 0;

    req_list[j].aiocb_p->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    req_list[j].aiocb_p->aio_sigevent.sigev_signo = IO_SIGNAL;
    req_list[j].aiocb_p->aio_sigevent.sigev_value.sival_ptr = &fin_mask[j];

    conditional_handle_error(aio_read(req_list[j].aiocb_p) == -1,
                             "aio_read error");
  }

  uint32_t num_finished = 0;
  while (num_finished != num_files) {
    for (uint32_t i = 0; i < num_files; ++i) {
      if (fin_mask[i] == 1 && processed_mask[i] == 0) {
        ++num_finished;
        processed_mask[i] = 1;
        req_list[i].status = aio_error(req_list[i].aiocb_p);

        if (req_list[i].status != 0) {
          perror("aio_error");
        }

        // process
        char *line = (char *)req_list[i].aiocb_p->aio_buf;
        uint32_t line_length = req_list[i].aiocb_p->aio_nbytes;
        line[line_length - 1] = '\0';

        coro_spawn_to_sort(line, line_length);
      }
    }
  }

  for (uint32_t j = 0; j < num_files; ++j) {
    free((void *)req_list[j].aiocb_p->aio_buf);
  }

  free(processed_mask);
  free(fin_mask);
  free(aiocb_list);
  free(req_list);
}

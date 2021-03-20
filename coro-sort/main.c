#include <fcntl.h>
#include <unistd.h>

#include "aio_read_util.h"
#include "coroutine.h"
#include "merge_sorted_arrays.h"
#include "scheduler.h"

int main(int argc, char** argv) {
  ucontext_t main_ctx;

  uint32_t num_files = argc - 1;

  char** buffers = calloc(num_files, sizeof(char*));
  int32_t** arrays = calloc(num_files, sizeof(int32_t*));
  int32_t* sizes = calloc(num_files, sizeof(int32_t));

  make_scheduler(&global_coro_scheduler, &main_ctx, num_files + 1);

  struct ucontext_t* ctx = spawn_coroutine(&global_coro_scheduler);
  makecontext(ctx, (void (*)(void))aio_read_util, 5, num_files, argv + 1,
              buffers, arrays, sizes);

  conditional_handle_error(
      swapcontext(&main_ctx, global_coro_scheduler.scheduler_ctx),
      "error while yielding");

  destroy_scheduler(&global_coro_scheduler);

  uint32_t total_size = 0;
  for (uint32_t i = 0; i < num_files; ++i) {
    total_size += sizes[i];
  }

  int32_t* sort_res = calloc(total_size, sizeof(int32_t));

  merge_sorted_arrays(arrays, num_files, sizes, sort_res, total_size);

  int fd =
      open("../../output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);

  for (uint32_t i = 0; i < total_size; ++i) {
    dprintf(fd, "%d ", sort_res[i]);
  }
  close(fd);

  free(sort_res);

  for (uint32_t i = 0; i < num_files; ++i) {
    free(buffers[i]);
  }
  free(buffers);
  free(sizes);

  for (uint32_t i = 0; i < num_files; ++i) {
    free(arrays[i]);
  }
  free(arrays);

  return 0;
}

#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "aio_read_util.h"
#include "coroutine.h"
#include "merge_sorted_arrays.h"
#include "scheduler.h"

void print_separator() {
  for (size_t i = 0; i < 45; ++i) {
    printf("-");
  }
  printf("\n");
}

void print_description(uint32_t num_files) {
  printf("Coroutine #0 purpose: async read from %d files\n", num_files);

  for (uint32_t i = 1; i <= num_files; ++i) {
    printf("Coroutine #%d purpose: sort file #%d\n", i, i);
  }

  printf("Coroutine #%d purpose: merge %d sorted arrays\n", num_files + 1,
         num_files);

  print_separator();

  printf("Scheduling started twice:\n"
         "1) To read from files and sort arrays\n"
         "2) To merge sorted arrays\n"
         "Jumping to scheduler and\n"
         "reusing it(updating trampoline and stack)\n"
         "works same as waiting for previous coroutines\n"
         "to finish and then spawning new coroutines\n");
}

int main(int argc, char** argv) {
  uint32_t num_files = argc - 1;

  print_description(num_files);
  print_separator();

  ucontext_t main_ctx;

  clock_t cur_time = clock();

  char** buffers = calloc(num_files, sizeof(char*));
  conditional_handle_error(buffers == NULL, "buffers calloc error");

  int32_t** arrays = calloc(num_files, sizeof(int32_t*));
  conditional_handle_error(arrays == NULL, "arrays calloc error");

  int32_t* sizes = calloc(num_files, sizeof(int32_t));
  conditional_handle_error(sizes == NULL, "sizes calloc error");

  // creating scheduler
  make_scheduler(&global_coro_scheduler, &main_ctx, num_files + 2);

  // spawning coroutine for async reading
  struct ucontext_t* ctx = spawn_coroutine(&global_coro_scheduler);
  makecontext(ctx, (void (*)(void))aio_read_util, 5, num_files, argv + 1,
              buffers, arrays, sizes);

  // jumping to scheduler to start coroutines
  jump_to_scheduler(&global_coro_scheduler, &main_ctx);

  uint32_t total_size = 0;
  for (uint32_t i = 0; i < num_files; ++i) {
    total_size += sizes[i];
  }

  int32_t* sort_res = calloc(total_size, sizeof(int32_t));
  conditional_handle_error(sort_res == NULL,
                           "sort results calloc error");

  // reusing scheduler - updating trampoline and stack
  memset(&main_ctx, 0, sizeof(struct ucontext_t));
  reuse_scheduler(&global_coro_scheduler, &main_ctx);

  // spawning coroutine to merge sorted arrays
  struct ucontext_t* merge_coro_ctx = spawn_coroutine(&global_coro_scheduler);
  makecontext(merge_coro_ctx, (void (*)(void))merge_sorted_arrays, 5, arrays,
              num_files, sizes, sort_res, total_size);

  // jumping to scheduler to start coroutine
  jump_to_scheduler(&global_coro_scheduler, &main_ctx);

  int fd =
      open("../../output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);

  // outputting sort results
  for (uint32_t i = 0; i < total_size; ++i) {
    dprintf(fd, "%d ", sort_res[i]);
  }
  close(fd);

  free(sort_res);

  print_separator();

  // print metrics
  print_scheduler_metrics(&global_coro_scheduler);

  // print overall time
  double total_time = ((double)(clock() - cur_time)) / CLOCKS_PER_SEC * 1000;
  printf("Program total time: %.1f ms\n", total_time);

  destroy_scheduler(&global_coro_scheduler);

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

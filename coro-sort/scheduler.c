#include "scheduler.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <ucontext.h>

static void *allocate_stack_sig() {
  void *stack = malloc(stack_size);
  stack_t ss;
  ss.ss_sp = stack;
  ss.ss_size = stack_size;
  ss.ss_flags = 0;
  sigaltstack(&ss, NULL);
  return stack;
}

static void *allocate_stack_mmap() {
  return mmap(NULL, stack_size, PROT_READ | PROT_WRITE | PROT_EXEC,
              MAP_ANON | MAP_PRIVATE, -1, 0);
}

static void *allocate_stack_mprot() {
  void *stack = malloc(stack_size);
  mprotect(stack, stack_size, PROT_READ | PROT_WRITE | PROT_EXEC);
  return stack;
}

void *allocate_stack(enum stack_type t) {
  switch (t) {
    case STACK_MMAP:
      return allocate_stack_mmap();
    case STACK_SIG:
      return allocate_stack_sig();
    case STACK_MPROT:
      return allocate_stack_mprot();
  }
}

void schedule(struct scheduler *coro_scheduler) {
  while (1) {
    size_t num_running = 0;
    size_t num_suspended = 0;
    size_t running_index = 0;
    for (size_t i = 0; i < coro_scheduler->free_slot; ++i) {
      if (coro_scheduler->statuses[i] == RUNNING) {
        ++num_running;
        running_index = i;
      }
      if (coro_scheduler->statuses[i] == SUSPENDED) {
        ++num_suspended;
      }
    }

    conditional_handle_error(num_running > 1,
                             "number of running coroutines is more than one");

    if (num_running == 0 && num_suspended == 0) {
      printf("scheduling finished\n");
      break;
    }

    if (num_running != 0) {
      coro_scheduler->statuses[running_index] = SUSPENDED;
    }

    size_t new_index = running_index;
    for (size_t i = 0; i < coro_scheduler->free_slot; ++i) {
      if (coro_scheduler->statuses[i] == SUSPENDED && running_index != i) {
        new_index = i;

        break;
      }
    }

    coro_scheduler->statuses[new_index] = RUNNING;
    coro_scheduler->running_coroutine = new_index;

    //    printf("Launched: %zu\n", coro_scheduler->running_coroutine);

    conditional_handle_error(
        swapcontext(coro_scheduler->scheduler_ctx,
                    &coro_scheduler->coroutines[new_index]),
        "error while switching to coroutine");
  }
}

void make_scheduler(struct scheduler *coro_scheduler,
                    struct ucontext_t *main_ctx, size_t max_coroutines) {
  memset(coro_scheduler, 0, sizeof(struct scheduler));

  coro_scheduler->free_slot = 0;
  coro_scheduler->max_coroutines = max_coroutines;

  char *scheduler_stack = allocate_stack(STACK_SIG);

  coro_scheduler->scheduler_ctx = malloc(sizeof(struct ucontext_t));
  conditional_handle_error(getcontext(coro_scheduler->scheduler_ctx) == -1,
                           "error while getting context");

  coro_scheduler->scheduler_ctx->uc_stack.ss_sp = scheduler_stack;
  coro_scheduler->scheduler_ctx->uc_stack.ss_size = stack_size;
  coro_scheduler->scheduler_ctx->uc_link = main_ctx;

  makecontext(coro_scheduler->scheduler_ctx, (void (*)(void))schedule, 1,
              coro_scheduler);

  coro_scheduler->coroutines =
      malloc(max_coroutines * sizeof(struct ucontext_t));
  coro_scheduler->statuses = malloc(max_coroutines * sizeof(enum coro_status));
}

void destroy_scheduler(struct scheduler *coro_scheduler) {
  for (size_t i = 0; i < coro_scheduler->free_slot; ++i) {
    free(coro_scheduler->coroutines[i].uc_link->uc_stack.ss_sp);
    free(coro_scheduler->coroutines[i].uc_link);
    free(coro_scheduler->coroutines[i].uc_stack.ss_sp);
  }

  free(coro_scheduler->coroutines);
  free(coro_scheduler->statuses);

  free(coro_scheduler->scheduler_ctx->uc_stack.ss_sp);
  free(coro_scheduler->scheduler_ctx);
}

void yield(struct scheduler *coro_scheduler) {
  conditional_handle_error(
      swapcontext(
          &coro_scheduler->coroutines[coro_scheduler->running_coroutine],
          coro_scheduler->scheduler_ctx),
      "error while yielding");
}

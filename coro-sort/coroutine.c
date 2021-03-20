#include "coroutine.h"

#include <string.h>

void finish_coroutine(struct scheduler* coro_scheduler) {
  coro_scheduler->statuses[coro_scheduler->running_coroutine] = FINISHED;
}

struct ucontext_t* spawn_coroutine(struct scheduler* coro_scheduler) {
  conditional_handle_error(
      coro_scheduler->free_slot == coro_scheduler->max_coroutines,
      "scheduler has no space to spawn coroutine");

  // making coroutine finish context
  char* finish_stack = allocate_stack(STACK_SIG);

  struct ucontext_t* finish_ctx = malloc(sizeof(struct ucontext_t));
  getcontext(finish_ctx);

  finish_ctx->uc_stack.ss_sp = finish_stack;
  finish_ctx->uc_stack.ss_size = stack_size;
  finish_ctx->uc_link = coro_scheduler->scheduler_ctx;
  makecontext(finish_ctx, (void (*)(void))finish_coroutine, 1, coro_scheduler);

  // making coroutine context
  char* job_stack = allocate_stack(STACK_SIG);

  coro_scheduler->statuses[coro_scheduler->free_slot] = SUSPENDED;

  struct ucontext_t* ctx =
      &coro_scheduler->coroutines[coro_scheduler->free_slot];
  memset(ctx, 0, sizeof(struct ucontext_t));
  conditional_handle_error(getcontext(ctx) == -1,
                           "error while getting context");

  ctx->uc_stack.ss_sp = job_stack;
  ctx->uc_stack.ss_size = stack_size;
  ctx->uc_link = finish_ctx;

  ++coro_scheduler->free_slot;

  return ctx;
}

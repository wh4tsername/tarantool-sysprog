#ifndef TARANTOOL_SYSPROG_CORO_SORT_SCHEDULER_H_
#define TARANTOOL_SYSPROG_CORO_SORT_SCHEDULER_H_

#include <ucontext.h>

#include "defines.h"

#define stack_size 1024 * 1024

enum stack_type { STACK_MMAP, STACK_SIG, STACK_MPROT };

void* allocate_stack(enum stack_type t);

enum coro_status { SUSPENDED, RUNNING, FINISHED };

struct scheduler {
  ucontext_t* scheduler_ctx;
  struct ucontext_t* coroutines;
  enum coro_status* statuses;
  clock_t* cur_launch_time;
  clock_t* total_time;
  size_t free_slot;
  size_t max_coroutines;
  size_t running_coroutine;
};

void reuse_scheduler(struct scheduler* coro_scheduler,
                     struct ucontext_t* main_ctx);

void make_scheduler(struct scheduler* coro_scheduler,
                    struct ucontext_t* main_ctx, size_t max_coroutines);

void destroy_scheduler(struct scheduler* coro_scheduler);

void jump_to_scheduler(struct scheduler* coro_scheduler, ucontext_t* main_ctx);

void yield(struct scheduler* coro_scheduler);

void print_scheduler_metrics(struct scheduler* coro_scheduler);

// global scheduler
struct scheduler global_coro_scheduler;

#endif  // TARANTOOL_SYSPROG_CORO_SORT_SCHEDULER_H_

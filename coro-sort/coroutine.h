#ifndef TARANTOOL_SYSPROG_CORO_SORT_COROUTINE_H_
#define TARANTOOL_SYSPROG_CORO_SORT_COROUTINE_H_

#include "defines.h"
#include "scheduler.h"

struct ucontext_t* spawn_coroutine(struct scheduler* coro_scheduler);

#endif  // TARANTOOL_SYSPROG_CORO_SORT_COROUTINE_H_

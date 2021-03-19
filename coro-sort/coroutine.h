#ifndef TARANTOOL_SYSPROG_CORO_SORT_COROUTINE_H_
#define TARANTOOL_SYSPROG_CORO_SORT_COROUTINE_H_

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <ucontext.h>

#include "defines.h"

#define stack_size 1024 * 1024

enum stack_type { STACK_MMAP, STACK_SIG, STACK_MPROT };

void *allocate_stack(enum stack_type t);

#endif // TARANTOOL_SYSPROG_CORO_SORT_COROUTINE_H_

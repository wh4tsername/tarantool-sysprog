#ifndef TARANTOOL_SYSPROG_CORO_SORT_DEFINES_H_
#define TARANTOOL_SYSPROG_CORO_SORT_DEFINES_H_

#include <stdio.h>
#include <stdlib.h>

#define conditional_handle_error(stmt, msg) \
  if (stmt) {                               \
    fprintf(stderr, "%s", msg);             \
    exit(EXIT_FAILURE);                     \
  }

#define handle_error(msg) \
  do {                    \
    perror(msg);          \
    exit(EXIT_FAILURE);   \
  } while (0)

#endif  // TARANTOOL_SYSPROG_CORO_SORT_DEFINES_H_

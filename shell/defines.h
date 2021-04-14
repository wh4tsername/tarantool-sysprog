#ifndef TARANTOOL_SYSPROG_SHELL_DEFINES_H_
#define TARANTOOL_SYSPROG_SHELL_DEFINES_H_

#include <stdio.h>
#include <stdlib.h>

#define conditional_handle_error(stmt, msg) \
  if (stmt) {                               \
    panic(msg);                             \
  }

#define panic(msg)                          \
  {                                         \
  fprintf(stderr, "%s", msg);               \
  exit(EXIT_FAILURE);                       \
  }

#endif //TARANTOOL_SYSPROG_SHELL_DEFINES_H_

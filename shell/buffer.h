#ifndef TARANTOOL_SYSPROG_SHELL_BUFFER_H_
#define TARANTOOL_SYSPROG_SHELL_BUFFER_H_

#include <stdlib.h>

struct buffer {
  char* ptr;
  size_t cur_size;
};

void init_buffer(struct buffer* buf);

void free_buffer(struct buffer* buf);

#endif //TARANTOOL_SYSPROG_SHELL_BUFFER_H_

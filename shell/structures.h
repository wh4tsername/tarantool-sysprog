#ifndef TARANTOOL_SYSPROG_SHELL_STRUCTURES_H_
#define TARANTOOL_SYSPROG_SHELL_STRUCTURES_H_

#include <stdbool.h>
#include <stdlib.h>

#include "vector.h"

struct cmd {
  char* name;
  struct vector* argv;

  bool has_pipe_next;
  bool has_redir;
  bool append;

  const char* dest;
};

void create_cmd(struct cmd* cmd);
void clear_cmd(struct cmd* cmd);

struct pipeline {
  struct vector* commands;

  bool has_combinator_next;
  bool is_and;
};

void create_pipeline(struct pipeline* pipeline);
void clear_pipeline(struct pipeline* pipeline);

struct query {
  struct vector* pipelines;
  bool in_background;
};

void create_query(struct query* query);
void clear_query(struct query* query);

#endif //TARANTOOL_SYSPROG_SHELL_STRUCTURES_H_

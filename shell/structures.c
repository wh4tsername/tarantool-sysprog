#include "structures.h"
#include "defines.h"

void create_cmd(struct cmd* cmd) {
  cmd->name = NULL;
  cmd->argv = calloc(1, sizeof(struct vector));
  conditional_handle_error(cmd->argv == NULL, "calloc argv error");

  create_vector(cmd->argv, sizeof(char*));

  cmd->has_pipe_next = false;
  cmd->has_redir = false;
  cmd->append = false;

  cmd->dest = NULL;
}

void clear_cmd(struct cmd* cmd) {
  for (size_t i = 0; i < cmd->argv->cur_size; ++i) {
    free(get_at(cmd->argv, char*, i));
  }

  destroy_vector(cmd->argv);
  free(cmd->argv);

  cmd->argv = NULL;
  cmd->name = NULL;

  cmd->has_pipe_next = false;
  cmd->has_redir = false;
  cmd->append = false;

  cmd->dest = NULL;
}

void create_pipeline(struct pipeline* pipeline) {
  pipeline->commands = calloc(1, sizeof(struct vector));
  conditional_handle_error(pipeline->commands == NULL, "calloc commands error");

  create_vector(pipeline->commands, sizeof(struct cmd*));

  pipeline->has_combinator_next = false;
  pipeline->is_and = false;
}

void clear_pipeline(struct pipeline* pipeline) {
  for (size_t i = 0; i < pipeline->commands->cur_size; ++i) {
    struct cmd* command = get_at(pipeline->commands, struct cmd*, i);

    clear_cmd(command);
    free(command);
  }

  destroy_vector(pipeline->commands);
  free(pipeline->commands);
  pipeline->commands = NULL;

  pipeline->has_combinator_next = false;
  pipeline->is_and = false;
}

void create_query(struct query* query) {
  query->pipelines = calloc(1, sizeof(struct vector));
  conditional_handle_error(query->pipelines == NULL, "calloc pipelines error");

  create_vector(query->pipelines, sizeof(struct pipeline*));

  query->in_background = false;
}

void clear_query(struct query* query) {
  for (size_t i = 0; i < query->pipelines->cur_size; ++i) {
    struct pipeline* pipeline = get_at(query->pipelines, struct pipeline*, i);

    clear_pipeline(pipeline);
    free(pipeline);
  }

  destroy_vector(query->pipelines);
  free(query->pipelines);

  query->in_background = false;
}

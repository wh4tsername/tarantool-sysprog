#include "client.h"
#include "buffer.h"
#include "defines.h"
#include "structures.h"
#include "vector.h"
#include "handlers.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ssize_t get_line(struct buffer* buf) {
  struct vector vec;

  bool in_string = false;
  char quote = ' ';
  bool lazy_alloc = true;
  while (true) {
    // scan symbol
    int res = getchar();

    if (res == -1) {
      return res;
    }

    if (lazy_alloc) {
      create_vector(&vec, sizeof(char));

      lazy_alloc = false;
    }

    char symbol = (char)res;

    // handle quotes
    if (symbol == '"') {
      if (!in_string) {
        in_string = true;
        quote = '"';
      } else if (symbol == quote) {
        in_string = false;
      }
    }
    if (symbol == '\'') {
      if (!in_string) {
        in_string = true;
        quote = '\'';
      } else if (symbol == quote) {
        in_string = false;
      }
    }

    if (in_string && symbol == '\n') {
      emplace_back(&vec, &symbol);
      continue;
    }

    // stop reading
    if (symbol == '\n') {
      break;
    }

    // handle slash
    if (symbol == '\\') {
      symbol = (char)getchar();
      if (symbol == '\n') {
        continue;
      } else {
        char slash = '\\';
        emplace_back(&vec, &slash);

        emplace_back(&vec, &symbol);
      }
    } else {
      emplace_back(&vec, &symbol);
    }
  }

  // form string
  char symbol = '\0';
  emplace_back(&vec, &symbol);

  buf->ptr = vec.ptr;
  buf->cur_size = vec.cur_size;

  return (ssize_t)buf->cur_size;
}

bool read_cmd(struct buffer* buf) {
//  printf("mini-shell$ ");

  ssize_t num_read_bytes = get_line(buf);
  if (num_read_bytes <= 0) {
    free_buffer(buf);

    return false;
  }

  return true;
}

void handle_amp(struct buffer* buf,
                struct vector* vec,
                size_t* i) {
  if (*i != buf->cur_size + 1 && buf->ptr[*i + 1] == '&') {
    push_back_str(vec, "&&");

    *i += 2;
  } else {
    push_back_str(vec, "&");

    *i += 1;
  }
}

void handle_pipe(struct buffer* buf,
                 struct vector* vec,
                 size_t* i) {
  if (*i == buf->cur_size + 1) {
    panic("unknown token | in the end");
  } else {
    if (buf->ptr[*i + 1] == '|') {
      push_back_str(vec, "||");

      *i += 2;
    } else {
      push_back_str(vec, "|");

      *i += 1;
    }
  }
}

void handle_angle(struct buffer* buf,
                  struct vector* vec,
                  size_t* i) {
  if (*i == buf->cur_size + 1) {
    panic("unknown token > in the end");
  } else {
    if (buf->ptr[*i + 1] == '>') {
      push_back_str(vec, ">>");

      *i += 2;
    } else {
      push_back_str(vec, ">");

      *i += 1;
    }
  }
}

void handle_quotes(struct buffer* buf,
                   struct vector* vec,
                   size_t* i,
                   char quote) {
  size_t j = 1;
  bool second_quote_found = false;
  while (*i + j < buf->cur_size) {
    if (buf->ptr[*i + j] == quote) {
      second_quote_found = true;
      break;
    } else if (buf->ptr[*i + j] == '\\') {
      j += 2;
    } else {
      ++j;
    }
  }

  conditional_handle_error(!second_quote_found,
                           "second quote is missing");

  // delete slashes and form token
  char* token = calloc(j + 1, sizeof(char));
  size_t num_slashes = 0;
  bool last_was_slash = false;
  for (size_t k = 1; k < j; ++k) {
    if (buf->ptr[*i + k] != '\\' || last_was_slash) {
      token[k - num_slashes - 1] = buf->ptr[*i + k];
      last_was_slash = false;
    } else {
      ++num_slashes;
      last_was_slash = true;
    }
  }
  token[j - num_slashes] = '\0';

  emplace_back(vec, &token);

  *i += j + 1;
}

bool is_not_control_symbol(char symbol) {
  return symbol != '&' && symbol != '|' && symbol != '>' &&
    symbol != '\'' && symbol != '"' && symbol != ' ' &&
    symbol != '\t' && symbol != '\n' && symbol != '\\';
}

void handle_str(struct buffer* buf,
                struct vector* vec,
                size_t* i) {
  size_t j = 0;
  while (*i + j < buf->cur_size) {
    if (is_not_control_symbol(buf->ptr[*i + j])) {
      ++j;
    } else if (buf->ptr[*i + j] == '\\') {
      j += 2;
    } else {
      break;
    }
  }

  // delete slashes form token
  char* token = calloc(j + 1, sizeof(char));
  size_t num_slashes = 0;
  bool last_was_slash = false;
  for (size_t k = 0; k < j; ++k) {
    if (buf->ptr[*i + k] != '\\' || last_was_slash) {
      token[k - num_slashes] = buf->ptr[*i + k];
      last_was_slash = false;
    } else {
      ++num_slashes;
      last_was_slash = true;
    }
  }
  token[j - num_slashes] = '\0';

  emplace_back(vec, &token);

  *i += j;
}

void parse(struct buffer* buf, struct vector* vec) {
  // check for comment
  char* comment_ptr = strtok(buf->ptr, "#");
  buf->cur_size = comment_ptr == buf->ptr + 1 || comment_ptr == NULL ? 0 : strlen(comment_ptr);

  // tokenizing
  for (size_t i = 0; i < buf->cur_size;) {
    if (buf->ptr[i] == '&') {
      handle_amp(buf, vec, &i);
    } else if (buf->ptr[i] == '|') {
      handle_pipe(buf, vec, &i);
    } else if (buf->ptr[i] == '>') {
      handle_angle(buf, vec, &i);
    } else if (buf->ptr[i] == '"') {
      handle_quotes(buf, vec, &i, '"');
    } else if (buf->ptr[i] == '\'') {
      handle_quotes(buf, vec, &i, '\'');
    } else if (buf->ptr[i] == ' ' || buf->ptr[i] == '\t' || buf->ptr[i] == '\n') {
      ++i;
    } else if (is_not_control_symbol(buf->ptr[i]) || buf->ptr[i] == '\\') {
      handle_str(buf, vec, &i);
    } else {
      panic("unreachable branch");
    }
  }
}

void flush_pipeline(struct query* query, struct pipeline** pipeline, bool* pipeline_needs_flush) {
  conditional_handle_error(!*pipeline_needs_flush, "flush flag invariant failure");

  emplace_back(query->pipelines, pipeline);

  // init new pipeline
  *pipeline = calloc(1, sizeof(struct pipeline));
  create_pipeline(*pipeline);
  *pipeline_needs_flush = false;
}

void flush_command(struct pipeline* pipeline, struct cmd** command, bool* command_needs_flush) {
  conditional_handle_error(!*command_needs_flush, "flush flag invariant failure");

  emplace_back(pipeline->commands, command);

  // init new command
  *command = calloc(1, sizeof(struct cmd));
  create_cmd(*command);
  *command_needs_flush = false;
}

void form_query(struct vector* vec, struct query* query) {
  // init command
  struct cmd* command = calloc(1, sizeof(struct cmd));
  conditional_handle_error(command == NULL, "calloc command error");
  create_cmd(command);

  // init pipeline
  struct pipeline* pipeline = calloc(1, sizeof(struct pipeline));
  conditional_handle_error(pipeline == NULL, "calloc pipeline error");
  create_pipeline(pipeline);

  bool command_needs_flush = false;
  bool pipeline_needs_flush = false;
  for (size_t i = 0; i < vec->cur_size; ++i) {
    if (strcmp("&&", get_at(vec, char*, i)) == 0) {
      pipeline->has_combinator_next = true;
      pipeline->is_and = true;

      flush_command(pipeline, &command, &command_needs_flush);
      flush_pipeline(query, &pipeline, &pipeline_needs_flush);
    } else if (strcmp("&", get_at(vec, char*, i)) == 0) {
      query->in_background = true;

      flush_command(pipeline, &command, &command_needs_flush);
      flush_pipeline(query, &pipeline, &pipeline_needs_flush);
    } else if (strcmp("||", get_at(vec, char*, i)) == 0) {
      pipeline->has_combinator_next = true;

      flush_command(pipeline, &command, &command_needs_flush);
      flush_pipeline(query, &pipeline, &pipeline_needs_flush);
    } else if (strcmp("|", get_at(vec, char*, i)) == 0) {
      command->has_pipe_next = true;

      flush_command(pipeline, &command, &command_needs_flush);
    } else if (strcmp(">>", get_at(vec, char*, i)) == 0) {
      command->has_redir = true;
      command->append = true;

      command->dest = get_at(vec, char*, i + 1);
      ++i;
    } else if (strcmp(">", get_at(vec, char*, i)) == 0) {
      command->has_redir = true;

      command->dest = get_at(vec, char*, i + 1);
      ++i;
    } else {
      if (!command_needs_flush) {
        command->name = get_at(vec, char*, i);

        command_needs_flush = true;
        pipeline_needs_flush = true;
      } else {
        push_back_str(command->argv, get_at(vec, char*, i));
      }
    }
  }

  if (command_needs_flush) {
    flush_command(pipeline, &command, &command_needs_flush);
    flush_pipeline(query, &pipeline, &pipeline_needs_flush);
  } else if (pipeline_needs_flush) {
    flush_pipeline(query, &pipeline, &pipeline_needs_flush);
  }

  clear_cmd(command);
  free(command);

  clear_pipeline(pipeline);
  free(pipeline);
}

void run_shell() {
  struct buffer buf;
  init_buffer(&buf);

  while (read_cmd(&buf)) {
    // parse command
    struct vector vec;
    create_vector(&vec, sizeof(char*));

    parse(&buf, &vec);
//    free_buffer(&buf);

    // form query
    struct query query;
    create_query(&query);

    form_query(&vec, &query);

    // execute query
    execute(&query);

    clear_query(&query);

    // reinit buffer
    free_buffer(&buf);
//    init_buffer(&buf);

    for (size_t i = 0; i < vec.cur_size; ++i) {
      free(get_at(&vec, char*, i));
    }
    destroy_vector(&vec);
  }
}

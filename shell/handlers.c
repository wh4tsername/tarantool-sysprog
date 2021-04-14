#include "handlers.h"
#include "defines.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

int execute_cd(struct cmd* command) {
  if (command->argv->cur_size == 1) {
    char* path_str = get_at(command->argv, char*, 0);
    DIR* path = opendir(path_str);

    if (path == NULL || chdir(path_str) != 0) {
      return 1;
    }

    closedir(path);

    return 0;
  } else {
    return 1;
  }
}

void execute_command(struct cmd* command, char** argv) {
  int ret = execvp(command->name, argv);
//  dprintf(STDERR_FILENO, "%s\n", strerror(errno));

  exit(ret);
}

int execute_pipeline(struct pipeline* pipeline) {
  int status = 0;
  int pipe_in = 0;
  for (size_t i = 0; i < pipeline->commands->cur_size; ++i) {
    struct cmd* command = get_at(pipeline->commands, struct cmd*, i);

    if (strcmp(command->name, "cd") == 0) {
      int ret = execute_cd(command);
      if (ret != 0) {
        dprintf(STDERR_FILENO, "No such file or directory\n");
      }

      continue;
    } else if (strcmp(command->name, "quit") == 0) {
      exit(0);
    }

    char** argv = calloc(command->argv->cur_size + 2, sizeof(char*));
    argv[0] = command->name;
    for (size_t j = 0; j < command->argv->cur_size; ++j) {
      argv[j + 1] = get_at(command->argv, char*, j);
    }
    argv[command->argv->cur_size + 1] = NULL;

    if (command->has_pipe_next) {
      int fds[2];
      conditional_handle_error(pipe(fds) == -1, "pipe creation error");

      pid_t pid = fork();
      conditional_handle_error(pid == -1, "fork failure");
      if (pid == 0) {
        dup2(pipe_in, 0);
        if (i != pipeline->commands->cur_size - 1) {
          dup2(fds[1], 1);
        }

        close(fds[0]);
        close(fds[1]);

        execute_command(command, argv);
      } else {
        conditional_handle_error(waitpid(pid, &status, 0) == -1,
                                 "wait failure");

        close(fds[1]);
        pipe_in = fds[0];
      }
    } else if (command->has_redir) {
      int flag = O_WRONLY | O_CREAT | (command->append ? O_APPEND : O_TRUNC);

      int fd = open(command->dest, flag, 0644);
      conditional_handle_error(fd == -1,
                               "can't open file to redirect to");

      pid_t pid = fork();
      conditional_handle_error(pid == -1, "fork failure");
      if (pid == 0) {
        dup2(pipe_in, 0);
        dup2(fd, 1);
        close(fd);
        close(pipe_in);

        execute_command(command, argv);
      } else {
        close(fd);

        conditional_handle_error(waitpid(pid, &status, 0) == -1,
                                 "wait failure");
      }
    } else {
      pid_t pid = fork();
      conditional_handle_error(pid == -1, "fork failure");
      if (pid == 0) {
        dup2(pipe_in, 0);
        close(pipe_in);

        execute_command(command, argv);
      } else {
        conditional_handle_error(waitpid(pid, &status, 0) == -1,
                                 "wait failure");
      }
    }

    free(argv);
  }

  return WEXITSTATUS(status);
}

int execute_query(struct query* query) {
  int prev_return_code = 0;
  bool skip_next = false;
  for (size_t i = 0; i < query->pipelines->cur_size; ++i) {
    if (skip_next) {
      skip_next = false;
      continue;
    }

    struct pipeline* pipeline = get_at(query->pipelines,struct pipeline*, i);

    prev_return_code = execute_pipeline(pipeline);
    if (!pipeline->has_combinator_next ||
        (pipeline->has_combinator_next &&
          pipeline->is_and &&
          prev_return_code != 0) ||
        (pipeline->has_combinator_next &&
          !pipeline->is_and &&
          prev_return_code == 0)) {
      skip_next = true;
    }
  }

  return prev_return_code;
}

void execute(struct query* query) {
  if (query->in_background) {
    int fds[2];
    conditional_handle_error(pipe(fds) == -1, "pipe creation error");

    pid_t pid = fork();
    conditional_handle_error(pid == -1, "fork failure");
    if (pid == 0) {
      // child
      exit(execute_query(query));
    } else {
      // parent
      // dprintf(STDOUT_FILENO, "[1] %d\n", pid);
    }
  } else {
    execute_query(query);
  }
}

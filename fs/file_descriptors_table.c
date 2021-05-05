#include "file_descriptors_table.h"

#include <stdlib.h>

#include "fs_defines.h"

int init_fd_table(struct file_descriptors_table* fd_table) {
  fd_table->file_descriptor_capacity = 1;
  fd_table->file_descriptor_count = 0;

  fd_table->file_descriptors = reallocarray(
      NULL, fd_table->file_descriptor_capacity, sizeof(struct filedesc*));
  conditional_error(fd_table->file_descriptors == NULL);

  fd_table->file_descriptors[0] = NULL;

  return 0;
}

int resize_fd_table(struct file_descriptors_table* fd_table) {
  if (fd_table->file_descriptor_capacity == fd_table->file_descriptor_count) {
    fd_table->file_descriptor_capacity *= 2;

    fd_table->file_descriptors = reallocarray(
        fd_table->file_descriptors, fd_table->file_descriptor_capacity,
        sizeof(struct filedesc*));
    conditional_error(fd_table->file_descriptors == NULL);

    for (int i = fd_table->file_descriptor_count;
         i < fd_table->file_descriptor_capacity; ++i) {
      fd_table->file_descriptors[i] = NULL;
    }
  }

  return 0;
}

int occupy_fd(struct file_descriptors_table* fd_table, struct filedesc** fd) {
  conditional_error(resize_fd_table(fd_table) == -1);

  for (int i = 0; i < fd_table->file_descriptor_capacity; ++i) {
    if (fd_table->file_descriptors[i] == NULL) {
      fd_table->file_descriptors[i] = calloc(1, sizeof(struct filedesc));
      conditional_error(fd_table->file_descriptors[i] == NULL);

      *fd = fd_table->file_descriptors[i];
      (*fd)->offset = 0;
      (*fd)->fd = i;

      ++fd_table->file_descriptor_count;

      return 0;
    }
  }

  return -1;
}

int get_at_fd(struct file_descriptors_table* fd_table, int fd,
              struct filedesc** fd_ptr) {
  conditional_error(fd >= fd_table->file_descriptor_capacity || fd < 0 ||
                    fd_table->file_descriptors[fd] == NULL);

  *fd_ptr = fd_table->file_descriptors[fd];

  return 0;
}

int free_fd(struct file_descriptors_table* fd_table, int fd) {
  for (int i = 0; i < fd_table->file_descriptor_capacity; ++i) {
    struct filedesc* descriptor = fd_table->file_descriptors[i];
    if (descriptor != NULL && descriptor->fd == fd) {
      free(descriptor);

      fd_table->file_descriptors[i] = NULL;

      --fd_table->file_descriptor_count;

      return 0;
    }
  }

  return -1;
}

void delete_fd_table(struct file_descriptors_table* fd_table) {
  free(fd_table->file_descriptors);

  fd_table->file_descriptors = NULL;
  fd_table->file_descriptor_capacity = 0;
  fd_table->file_descriptor_count = 0;
}

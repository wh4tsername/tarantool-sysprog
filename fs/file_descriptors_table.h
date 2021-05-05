#ifndef TARANTOOL_SYSPROG_FS_FILE_DESCRIPTORS_TABLE_H_
#define TARANTOOL_SYSPROG_FS_FILE_DESCRIPTORS_TABLE_H_

#include "filedesc.h"

struct file_descriptors_table {
  struct filedesc** file_descriptors;
  int file_descriptor_count;
  int file_descriptor_capacity;
};

int init_fd_table(struct file_descriptors_table* fd_table);

int occupy_fd(struct file_descriptors_table* fd_table, struct filedesc** fd);

int get_at_fd(struct file_descriptors_table* fd_table, int fd,
              struct filedesc** fd_ptr);

int free_fd(struct file_descriptors_table* fd_table, int fd);

void delete_fd_table(struct file_descriptors_table* fd_table);

#endif  // TARANTOOL_SYSPROG_FS_FILE_DESCRIPTORS_TABLE_H_

#ifndef TARANTOOL_SYSPROG_FS_FILE_H_
#define TARANTOOL_SYSPROG_FS_FILE_H_

#include "block.h"

#include <stdbool.h>

#define MAX_FILE_SIZE (1024 * 1024 * 1024)

struct file {
  /** Double-linked list of file blocks. */
  struct block *block_list;
  /**
   * Last block in the list above for fast access to the end
   * of file.
   */
  struct block *last_block;
  /** How many file descriptors are opened on the file. */
  int refs;
  /** File name. */
  const char *name;
  /** Files are stored in a double-linked list. */
  struct file *next;
  struct file *prev;

  int file_size;

  bool del_flag;
};

int init_file(struct file *file_pointer, const char *filename);

int destroy_file(struct file *file_pointer);

#endif  // TARANTOOL_SYSPROG_FS_FILE_H_

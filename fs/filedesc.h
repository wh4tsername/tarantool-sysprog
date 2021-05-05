#ifndef TARANTOOL_SYSPROG_FS_FILEDESC_H_
#define TARANTOOL_SYSPROG_FS_FILEDESC_H_

#include "file.h"

struct filedesc {
  struct file *file;

  int fd;

  int offset;

  int flags;
};

#endif  // TARANTOOL_SYSPROG_FS_FILEDESC_H_

#ifndef TARANTOOL_SYSPROG_FS_FS_DEFINES_H_
#define TARANTOOL_SYSPROG_FS_FS_DEFINES_H_

#define conditional_set_error(expr, err) \
  if ((expr)) {                          \
    ufs_error_code = err;                \
    return -1;                           \
  }

#define conditional_error(expr) \
  if ((expr)) {                 \
    return -1;                  \
  }

#endif  // TARANTOOL_SYSPROG_FS_FS_DEFINES_H_

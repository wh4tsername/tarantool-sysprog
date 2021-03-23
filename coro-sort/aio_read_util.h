#ifndef TARANTOOL_SYSPROG_CORO_SORT_AIO_READ_UTIL_H_
#define TARANTOOL_SYSPROG_CORO_SORT_AIO_READ_UTIL_H_

#include <stdint.h>

void aio_read_util(uint32_t num_files, char** filenames, char** buffers,
                   int32_t** arrays, int32_t* sizes);

#endif  // TARANTOOL_SYSPROG_CORO_SORT_AIO_READ_UTIL_H_

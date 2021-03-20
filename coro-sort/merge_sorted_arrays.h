#ifndef TARANTOOL_SYSPROG_CORO_SORT_MERGE_SORTED_ARRAYS_H_
#define TARANTOOL_SYSPROG_CORO_SORT_MERGE_SORTED_ARRAYS_H_

#include <stdint.h>

void merge_sorted_arrays(int32_t** arrays, uint32_t num_arrays,
                         const int32_t* sizes, int32_t* sort_res,
                         uint32_t total_size);

#endif  // TARANTOOL_SYSPROG_CORO_SORT_MERGE_SORTED_ARRAYS_H_

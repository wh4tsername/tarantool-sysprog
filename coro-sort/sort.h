#ifndef TARANTOOL_SYSPROG_CORO_SORT_SORT_H_
#define TARANTOOL_SYSPROG_CORO_SORT_SORT_H_

#include <stdint.h>

void prepare_sort(char* number_line, uint32_t size, int32_t** array,
                  int32_t* count_numbers);

void sort(int32_t* array, uint32_t size);

#endif  // TARANTOOL_SYSPROG_CORO_SORT_SORT_H_

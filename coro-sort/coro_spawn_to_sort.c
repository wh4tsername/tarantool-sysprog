#include "coro_spawn_to_sort.h"

#include <stdio.h>
#include <stdlib.h>

#include "sort.h"

void coro_spawn_to_sort(char* number_line, uint32_t size) {
  const int max_number_length = 10;
  const uint32_t max_arr_size = (size + 1) / 2;

  char* buf = malloc(max_number_length);
  int32_t* arr = calloc(max_arr_size, sizeof(int32_t));

  char* new_ptr = NULL;
  uint32_t arr_size = 0;
  while (1) {
    int32_t num = strtol(number_line, &new_ptr, 10);

    arr[arr_size] = num;
    ++arr_size;

    if (*new_ptr == '\0') {
      break;
    }

    number_line = new_ptr;
    new_ptr = NULL;
  }

  sort(arr, arr_size);

  free(arr);
  free(buf);
}

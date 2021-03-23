#include "sort.h"

#include <stdlib.h>

#include "scheduler.h"

uint32_t min(uint32_t lhs, uint32_t rhs) { return lhs <= rhs ? lhs : rhs; }

void merge(int32_t* arr, int32_t* buf, uint32_t left, uint32_t mid,
           uint32_t right) {
  uint32_t it1 = 0;
  uint32_t it2 = 0;

  while (left + it1 < mid && mid + it2 < right) {
    if (arr[left + it1] < arr[mid + it2]) {
      buf[it1 + it2] = arr[left + it1];
      ++it1;
    } else {
      buf[it1 + it2] = arr[mid + it2];
      ++it2;
    }
  }

  while (left + it1 < mid) {
    buf[it1 + it2] = arr[left + it1];
    ++it1;
  }

  while (mid + it2 < right) {
    buf[it1 + it2] = arr[mid + it2];
    ++it2;
  }

  for (uint32_t i = 0; i < it1 + it2; ++i) {
    arr[left + i] = buf[i];
  }
}

void sort(int32_t* array, uint32_t size) {
  // yielding because already spent time on preparing sort
  yield(&global_coro_scheduler);

  int32_t* buf = calloc(size, sizeof(int32_t));

  for (uint32_t i = 1; i < size; i *= 2) {
    for (uint32_t j = 0; j < size - i; j += 2 * i) {
      merge(array, buf, j, j + i, min(j + 2 * i, size));

      // yielding because spent time merging arrays
      yield(&global_coro_scheduler);
    }
  }

  free(buf);
}

void prepare_sort(char* number_line, uint32_t size, int32_t** array,
                  int32_t* count_numbers) {
  const int max_number_length = 10;
  const uint32_t max_arr_size = (size + 1) / 2;

  char* buf = malloc(max_number_length);
  *array = calloc(max_arr_size, sizeof(int32_t));

  char* new_ptr = NULL;
  *count_numbers = 0;
  while (*number_line != '\0') {
    int32_t num = strtol(number_line, &new_ptr, 10);

    (*array)[*count_numbers] = num;
    ++*count_numbers;

    number_line = new_ptr;
    new_ptr = NULL;
  }

  sort(*array, *count_numbers);

  free(buf);
}

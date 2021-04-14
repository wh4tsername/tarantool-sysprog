#include "sort.h"

#include <stdlib.h>

#include "scheduler.h"

#define YIELD yield(&global_coro_scheduler)

uint32_t min(uint32_t lhs, uint32_t rhs) { return lhs <= rhs ? lhs : rhs; }

void merge(int32_t* arr, int32_t* buf, uint32_t left, uint32_t mid,
           uint32_t right) {
  uint32_t it1 = 0; YIELD;
  uint32_t it2 = 0; YIELD;

  while (left + it1 < mid && mid + it2 < right) {
    if (arr[left + it1] < arr[mid + it2]) {
      buf[it1 + it2] = arr[left + it1]; YIELD;
      ++it1; YIELD;
    } else {
      buf[it1 + it2] = arr[mid + it2]; YIELD;
      ++it2; YIELD;
    }
  }

  while (left + it1 < mid) {
    buf[it1 + it2] = arr[left + it1]; YIELD;
    ++it1; YIELD;
  }

  while (mid + it2 < right) {
    buf[it1 + it2] = arr[mid + it2]; YIELD;
    ++it2; YIELD;
  }

  for (uint32_t i = 0; i < it1 + it2; ++i) {
    arr[left + i] = buf[i]; YIELD;
  }
}

void sort(int32_t* array, uint32_t size) {
  int32_t* buf = calloc(size, sizeof(int32_t)); YIELD;
  conditional_handle_error(buf == NULL, "sort buffer calloc error"); YIELD;

  for (uint32_t i = 1; i < size; i *= 2) {
    for (uint32_t j = 0; j < size - i; j += 2 * i) {
      merge(array, buf, j, j + i, min(j + 2 * i, size)); YIELD;
    }
  }

  free(buf); YIELD;
}

void prepare_sort(char* number_line, uint32_t size, int32_t** array,
                  int32_t* count_numbers) {
  const int max_number_length = 10; YIELD;
  const uint32_t max_arr_size = (size + 1) / 2; YIELD;

  char* buf = malloc(max_number_length); YIELD;
  conditional_handle_error(buf == NULL, "sort buffer calloc error"); YIELD;

  *array = calloc(max_arr_size, sizeof(int32_t)); YIELD;
  conditional_handle_error(*array == NULL, "sort array calloc error"); YIELD;

  char* new_ptr = NULL; YIELD;
  *count_numbers = 0; YIELD;
  while (*number_line != '\0') {
    int32_t num = (int32_t)strtol(number_line, &new_ptr, 10); YIELD;

    (*array)[*count_numbers] = num; YIELD;
    ++*count_numbers; YIELD;

    number_line = new_ptr; YIELD;
    new_ptr = NULL; YIELD;
  }

  sort(*array, *count_numbers); YIELD;

  free(buf); YIELD;
}

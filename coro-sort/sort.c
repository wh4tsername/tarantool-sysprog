#include "sort.h"

#include <stdlib.h>

uint32_t min(uint32_t lhs, uint32_t rhs) { return lhs <= rhs ? lhs : rhs; }

void merge(int32_t *arr, int32_t *buf, uint32_t left, uint32_t mid,
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

void sort(int32_t *arr, uint32_t size) {
  int32_t *buf = calloc(size, sizeof(int32_t));

  for (uint32_t i = 1; i < size; i *= 2) {
    for (uint32_t j = 0; j < size - i; j += 2 * i) {
      merge(arr, buf, j, j + i, min(j + 2 * i, size));
    }
  }

  free(buf);
}

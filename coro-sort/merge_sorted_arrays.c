#include "merge_sorted_arrays.h"

uint32_t index_of_min(const int32_t* elements, uint32_t num_elements) {
  uint32_t index = 0;

  for (uint32_t i = 1; i < num_elements; ++i) {
    if (elements[i] < elements[index]) {
      index = i;
    }
  }

  return index;
}

void merge_sorted_arrays(int32_t** arrays, uint32_t num_arrays,
                         const int32_t* sizes, int32_t* sort_res,
                         uint32_t total_size) {
  uint32_t indexes[num_arrays];
  int32_t merging_set[num_arrays];
  char usage_mask[num_arrays];

  for (uint32_t i = 0; i < num_arrays; ++i) {
    indexes[i] = 0;
    usage_mask[i] = 1;
  }

  for (uint32_t i = 0; i < total_size; ++i) {
    for (uint32_t j = 0; j < num_arrays; ++j) {
      if (indexes[j] >= sizes[j]) {
        usage_mask[j] = 0;
      }
    }

    uint32_t merging_set_size = 0;
    for (uint32_t j = 0; j < num_arrays; ++j) {
      if (usage_mask[j]) {
        merging_set[merging_set_size] = arrays[j][indexes[j]];
        ++merging_set_size;
      }
    }

    uint32_t min_index = index_of_min(merging_set, merging_set_size);

    sort_res[i] = merging_set[min_index];
    ++indexes[min_index];
  }
}

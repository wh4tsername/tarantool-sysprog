#include "vector.h"
#include "defines.h"

#include <string.h>

void create_vector(struct vector* vec, size_t size_of_elem) {
  vec->max_size = 1;
  vec->cur_size = 0;
  vec->size_of_elem = size_of_elem;

  vec->ptr = reallocarray(NULL, vec->max_size, vec->size_of_elem);
  conditional_handle_error(vec->ptr == NULL, "alloc vector error");
}

void resize_vector(struct vector* vec) {
  conditional_handle_error(vec->cur_size > vec->max_size,
                           "vector invariant failure");
  if (vec->cur_size == vec->max_size) {
    vec->max_size *= 2;

    vec->ptr = reallocarray(vec->ptr, vec->max_size, vec->size_of_elem);
    conditional_handle_error(vec->ptr == NULL,
                             "alloc vector error");
  }
}

void push_back_str(struct vector* vec, const char* str) {
  resize_vector(vec);

  char* temp = calloc(strlen(str) + 1, sizeof(char));
  conditional_handle_error(temp == NULL, "calloc string error");
  strcpy(temp, str);

  emplace_back(vec, &temp);
}

void emplace_back(struct vector* vec, void* elem) {
  resize_vector(vec);

  memcpy((char*)vec->ptr + vec->cur_size * vec->size_of_elem,
         elem,
         vec->size_of_elem);
  ++vec->cur_size;
}

void destroy_vector(struct vector* vec) {
  free(vec->ptr);

  vec->ptr = NULL;
  vec->max_size = 0;
  vec->cur_size = 0;
  vec->size_of_elem = 0;
}

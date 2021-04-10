#ifndef TARANTOOL_SYSPROG_SHELL_VECTOR_H_
#define TARANTOOL_SYSPROG_SHELL_VECTOR_H_

#include <stdlib.h>

struct vector {
  void* ptr;
  size_t max_size;
  size_t cur_size;
  size_t size_of_elem;
};

void create_vector(struct vector* vec, size_t size_of_elem);

void push_back_str(struct vector* vec, const char* str);

void emplace_back(struct vector* vec, void* elem);

#define get_at(vec, type, i) (*(type*) ((char*)(vec)->ptr + (i) * sizeof(type*)))

void destroy_vector(struct vector* vec);

#endif //TARANTOOL_SYSPROG_SHELL_VECTOR_H_

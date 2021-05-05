#include "file.h"

#include <stdlib.h>
#include <string.h>

#include "fs_defines.h"

int init_file(struct file* file_pointer, const char* filename) {
  file_pointer->block_list = NULL;
  file_pointer->last_block = NULL;

  file_pointer->refs = 0;

  size_t name_length = strlen(filename);
  file_pointer->name = malloc(name_length + 1);
  strcpy((char*)file_pointer->name, filename);

  file_pointer->next = NULL;
  file_pointer->prev = NULL;

  file_pointer->file_size = 0;

  file_pointer->del_flag = false;

  return 0;
}

int destroy_file(struct file* file_pointer) {
  file_pointer->block_list = NULL;

  conditional_error(destroy_chain(file_pointer->last_block));
  file_pointer->last_block = NULL;

  file_pointer->refs = 0;
  free((char*)file_pointer->name);

  file_pointer->next = NULL;
  file_pointer->prev = NULL;

  file_pointer->file_size = 0;

  file_pointer->del_flag = false;

  return 0;
}

#include "block.h"

#include <stdlib.h>

int init_block(struct block* block) {
  block->occupied = 0;
  block->next = NULL;
  block->prev = NULL;
  block->memory = calloc(1, BLOCK_SIZE);

  return block->memory == NULL ? -1 : 0;
}

int destroy_chain(struct block* last_block) {
  struct block* prev = NULL;
  struct block* next_to_delete = last_block;
  for (; next_to_delete != NULL; next_to_delete = next_to_delete->prev) {
    free(next_to_delete->memory);
    free(prev);

    prev = next_to_delete;
  }

  free(prev);

  return 0;
}

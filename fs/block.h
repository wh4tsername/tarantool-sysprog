#ifndef TARANTOOL_SYSPROG_FS_BLOCK_H_
#define TARANTOOL_SYSPROG_FS_BLOCK_H_

#define BLOCK_SIZE 512

struct block {
  /** Block memory. */
  char *memory;
  /** How many bytes are occupied. */
  int occupied;
  /** Next block in the file. */
  struct block *next;
  /** Previous block in the file. */
  struct block *prev;
};

int init_block(struct block* block);

int destroy_chain(struct block *last_block);

#endif  // TARANTOOL_SYSPROG_FS_BLOCK_H_

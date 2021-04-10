#include "buffer.h"

void init_buffer(struct buffer* buf) {
  buf->ptr = NULL;
  buf->cur_size = 0;
}

void free_buffer(struct buffer* buf) {
  free(buf->ptr);
  buf->ptr = NULL;
  buf->cur_size = 0;
}

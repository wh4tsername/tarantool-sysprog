#include "userfs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "file_descriptors_table.h"
#include "fs_defines.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/** Global error code. Set from any function on any error. */
static enum ufs_error_code ufs_error_code = UFS_ERR_NO_ERR;

/** List of all files. */
static struct file *file_list = NULL;

/** Descriptors table. */
static struct file_descriptors_table *fd_table = NULL;

int lazy_init() {
  if (fd_table == NULL) {
    fd_table = calloc(1, sizeof(struct file_descriptors_table));
    conditional_set_error(fd_table == NULL, UFS_ERR_NO_MEM);

    int ret = init_fd_table(fd_table);
    conditional_set_error(ret == -1, UFS_ERR_NO_MEM);
  }

  return UFS_ERR_NO_ERR;
}

/**
 * An array of file descriptors. When a file descriptor is
 * created, its pointer drops here. When a file descriptor is
 * closed, its place in this array is set to NULL and can be
 * taken by next ufs_open() call.
 */

enum ufs_error_code ufs_errno() { return ufs_error_code; }

bool find_file_by_name(struct file **res, const char *filename) {
  struct file *cur = file_list;
  bool found = false;
  while (cur != NULL) {
    if (strcmp(cur->name, filename) == 0) {
      found = true;
      *res = cur;
      break;
    }

    cur = cur->next;
  }

  return found;
}

int ufs_open(const char *filename, int flags) {
  // lazy init
  if (lazy_init() == -1) {
    return -1;
  }

  struct file *file_to_open = NULL;
  if (flags & UFS_CREATE) {
    if (find_file_by_name(&file_to_open, filename)) {
      file_to_open->del_flag = false;
      file_to_open->file_size = 0;
    } else {
      file_to_open = calloc(1, sizeof(struct file));
      conditional_set_error(file_to_open == NULL, UFS_ERR_NO_MEM);

      init_file(file_to_open, filename);

      if (file_list == NULL) {
        file_list = file_to_open;
      } else {
        struct file *prev_head = file_list;
        file_list = file_to_open;
        file_to_open->next = prev_head;
        prev_head->prev = file_to_open;
      }
    }
  } else {
    conditional_set_error(!find_file_by_name(&file_to_open, filename),
                          UFS_ERR_NO_FILE);

    conditional_set_error(file_to_open->del_flag, UFS_ERR_NO_FILE);
  }

  struct filedesc *fd = NULL;
  conditional_set_error(occupy_fd(fd_table, &fd) == -1, UFS_ERR_NO_MEM);

  if (!((fd->flags & UFS_WRITE_ONLY) || (fd->flags & UFS_READ_ONLY))) {
    fd->flags = flags | UFS_READ_WRITE;
  } else {
    fd->flags = flags;
  }

  fd->file = file_to_open;

  ++file_to_open->refs;

  return fd->fd;
}

ssize_t ufs_write(int fd, const char *buf, size_t size) {
  struct filedesc *fd_ptr = NULL;
  conditional_set_error(get_at_fd(fd_table, fd, &fd_ptr) == -1,
                        UFS_ERR_NO_FILE);

  conditional_set_error(fd_ptr->flags & UFS_READ_ONLY,
                        UFS_ERR_NO_PERMISSION);

  fd_ptr->offset = MIN(fd_ptr->offset, fd_ptr->file->file_size);

  ssize_t total_written = 0;

  if (fd_ptr->file->last_block == NULL) {
    struct block *new_block = calloc(1, sizeof(struct block));
    conditional_set_error(new_block == NULL, UFS_ERR_NO_MEM);
    conditional_set_error(init_block(new_block) == -1, UFS_ERR_NO_MEM);

    fd_ptr->file->block_list = new_block;
    fd_ptr->file->last_block = new_block;
  }

  struct block *cur_block = fd_ptr->file->block_list;
  for (int i = 0; i < fd_ptr->offset / BLOCK_SIZE; ++i) {
    if (cur_block->next == NULL) {
      struct block *new_block = calloc(1, sizeof(struct block));
      conditional_set_error(new_block == NULL, UFS_ERR_NO_MEM);
      conditional_set_error(init_block(new_block) == -1, UFS_ERR_NO_MEM);

      cur_block->next = new_block;
      new_block->prev = cur_block;

      cur_block = cur_block->next;

      break;
    }

    cur_block = cur_block->next;
  }

  int cur_offset = fd_ptr->offset % BLOCK_SIZE;

  while (total_written != size) {
    size_t cur_write =
        MIN(size - total_written, BLOCK_SIZE - cur_offset);

    conditional_set_error(fd_ptr->file->file_size + cur_write > MAX_FILE_SIZE, UFS_ERR_NO_MEM);

    memcpy(cur_block->memory + cur_offset, buf + total_written,
           cur_write);

    fd_ptr->file->file_size += (int)cur_write + cur_offset - cur_block->occupied;

    cur_block->occupied = (int)cur_write + cur_offset;

    if (cur_block->occupied == BLOCK_SIZE && cur_block->next == NULL) {
      struct block *new_block = calloc(1, sizeof(struct block));
      conditional_set_error(new_block == NULL, UFS_ERR_NO_MEM);
      conditional_set_error(init_block(new_block) == -1, UFS_ERR_NO_MEM);

      new_block->prev = cur_block;
      cur_block->next = new_block;

      if (fd_ptr->file->last_block == cur_block) {
        fd_ptr->file->last_block = new_block;
      }
    }

    fd_ptr->offset += (int)cur_write;

    total_written += (ssize_t)cur_write;
    cur_block = cur_block->next;

    cur_offset = 0;
  }

  return total_written;
}

ssize_t ufs_read(int fd, char *buf, size_t size) {
  struct filedesc *fd_ptr = NULL;
  conditional_set_error(get_at_fd(fd_table, fd, &fd_ptr) == -1,
                        UFS_ERR_NO_FILE);

  conditional_set_error(fd_ptr->flags & UFS_WRITE_ONLY,
                        UFS_ERR_NO_PERMISSION);

  if (fd_ptr->file->file_size - fd_ptr->offset >= 0) {
    size = MIN((int)size, fd_ptr->file->file_size - fd_ptr->offset);
  }

  ssize_t total_read = 0;

  struct block *cur_block = fd_ptr->file->block_list;
  for (int i = 0; i < fd_ptr->offset / BLOCK_SIZE; ++i) {
    cur_block = cur_block->next;
  }

  int cur_offset = fd_ptr->offset % BLOCK_SIZE;

  while (total_read != size) {
    size_t cur_read =
      MIN(size - total_read, BLOCK_SIZE - cur_offset);

    memcpy(buf + total_read, cur_block->memory + cur_offset, cur_read);

    fd_ptr->offset += (int)cur_read;

    total_read += (ssize_t)cur_read;
    cur_block = cur_block->next;

    cur_offset = 0;
  }

  return total_read;
}

int ufs_close(int fd) {
  struct filedesc *fd_ptr = NULL;
  conditional_set_error(get_at_fd(fd_table, fd, &fd_ptr) == -1,
                        UFS_ERR_NO_FILE);

  --fd_ptr->file->refs;

  if (fd_ptr->file->del_flag && fd_ptr->file->refs == 0) {
    ufs_delete(fd_ptr->file->name);
  }

  conditional_set_error(free_fd(fd_table, fd) == -1, UFS_ERR_NO_FILE);

  return UFS_ERR_NO_ERR;
}

int ufs_delete(const char *filename) {
  struct file *file_ptr = NULL;
  conditional_set_error(!find_file_by_name(&file_ptr, filename),
                        UFS_ERR_NO_FILE);

  if (file_ptr->refs != 0) {
//    file_ptr->file_size = 0;
    file_ptr->del_flag = true;
  } else {
    struct file *next = file_ptr->next;
    struct file *prev = file_ptr->prev;

    if (next != NULL) {
      next->prev = prev;
    }
    if (prev != NULL) {
      prev->next = next;
    }
    if (next == NULL && prev == NULL) {
      file_list = NULL;
    }
    if (file_list == file_ptr) {
      file_list = next;
    }

    destroy_file(file_ptr);

    free(file_ptr);
  }

  return UFS_ERR_NO_ERR;
}

int ufs_resize(int fd, size_t new_size) {
  struct filedesc *fd_ptr = NULL;
  conditional_set_error(get_at_fd(fd_table, fd, &fd_ptr) == -1,
                        UFS_ERR_NO_FILE);

  int old_size = fd_ptr->file->file_size;
  if (old_size < new_size) {
    int old_num = old_size / BLOCK_SIZE + 1;
    int new_num = (int)new_size / BLOCK_SIZE + 1;

    struct block* prev = fd_ptr->file->last_block;
    for (int i = 0; i < new_num - old_num; ++i) {
      struct block *new_block = calloc(1, sizeof(struct block));
      conditional_set_error(new_block == NULL, UFS_ERR_NO_MEM);
      conditional_set_error(init_block(new_block) == -1, UFS_ERR_NO_MEM);

      if (fd_ptr->file->last_block == NULL) {
        fd_ptr->file->block_list = new_block;
        fd_ptr->file->last_block = new_block;
      } else {
        prev->next = new_block;
        new_block->prev = prev;
        fd_ptr->file->last_block = new_block;
      }

      prev = new_block;
    }
  } else if (old_size > new_size) {
    conditional_set_error(new_size > MAX_FILE_SIZE, UFS_ERR_NO_MEM);

    int start_block = (int)new_size / BLOCK_SIZE;
    int end_block = old_size / BLOCK_SIZE;

    struct block* cur_block = fd_ptr->file->block_list;
    for (int i = 0; i < start_block; ++i) {
      cur_block = cur_block->next;
    }

    for (int i = start_block; i <= end_block; ++i) {
      int block_offset = i == start_block ? (int)new_size % BLOCK_SIZE : 0;

      memset(cur_block->memory + block_offset, 0, BLOCK_SIZE - block_offset);
      cur_block = cur_block->next;
    }
  }

  fd_ptr->file->file_size = (int)new_size;

  return 0;
}

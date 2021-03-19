#include "coroutine.h"

/**
 * Below you can see 3 different ways of how to allocate stack.
 * You can choose any. All of them do in fact the same.
 */

static void *allocate_stack_sig() {
  void *stack = malloc(stack_size);
  stack_t ss;
  ss.ss_sp = stack;
  ss.ss_size = stack_size;
  ss.ss_flags = 0;
  sigaltstack(&ss, NULL);
  return stack;
}

static void *allocate_stack_mmap() {
  return mmap(NULL, stack_size, PROT_READ | PROT_WRITE | PROT_EXEC,
              MAP_ANON | MAP_PRIVATE, -1, 0);
}

static void *allocate_stack_mprot() {
  void *stack = malloc(stack_size);
  mprotect(stack, stack_size, PROT_READ | PROT_WRITE | PROT_EXEC);
  return stack;
}

/**
 * Use this wrapper to choose your favourite way of stack
 * allocation.
 */
void *allocate_stack(enum stack_type t) {
  switch (t) {
    case STACK_MMAP:
      return allocate_stack_mmap();
    case STACK_SIG:
      return allocate_stack_sig();
    case STACK_MPROT:
      return allocate_stack_mprot();
  }
}

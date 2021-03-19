#include <unistd.h>

#include "aio_read_util.h"
#include "coroutine.h"

static ucontext_t uctx_main, uctx_func1, uctx_func2;

static void my_coroutine(int id) {
  printf("func%d: started\n", id);
  if (id == 1) {
    printf("coroutine1: swapcontext(&uctx_func1, &uctx_func2)\n");
    if (swapcontext(&uctx_func1, &uctx_func2) == -1)
      handle_error("swapcontext");
  } else {
    printf("coroutine2: swapcontext(&uctx_func2, &uctx_func1)\n");
    if (swapcontext(&uctx_func2, &uctx_func1) == -1)
      handle_error("swapcontext");
  }
  printf("func%d: returning\n", id);
}

int coro_test(int argc, char **argv) {
  /* First of all, create a stack for each coroutine. */
  char *func1_stack = allocate_stack(STACK_SIG);
  char *func2_stack = allocate_stack(STACK_MPROT);

  /*
   * Below is just initialization of coroutine structures.
   * They are not started yet. Just created.
   */
  if (getcontext(&uctx_func1) == -1) handle_error("getcontext");
  /*
   * Here you specify a stack, allocated earlier. Unique for
   * each coroutine.
   */
  uctx_func1.uc_stack.ss_sp = func1_stack;
  uctx_func1.uc_stack.ss_size = stack_size;
  /*
   * Important - here you specify, to which context to
   * switch after this coroutine is finished. The code below
   * says, that when 'uctx_func1' is finished, it should
   * switch to 'uctx_main'.
   */
  uctx_func1.uc_link = &uctx_main;
  makecontext(&uctx_func1, (void (*)(void))my_coroutine, 1, 1);

  if (getcontext(&uctx_func2) == -1) handle_error("getcontext");
  uctx_func2.uc_stack.ss_sp = func2_stack;
  uctx_func2.uc_stack.ss_size = stack_size;
  /* Successor context is f1(), unless argc > 1. */
  uctx_func2.uc_link = (argc > 1) ? NULL : &uctx_func1;
  makecontext(&uctx_func2, (void (*)(void))my_coroutine, 1, 2);

  /*
   * And here it starts. The first coroutine to start is
   * 'uctx_func2'.
   */
  printf("main: swapcontext(&uctx_main, &uctx_func2)\n");
  if (swapcontext(&uctx_main, &uctx_func2) == -1) handle_error("swapcontext");

  printf("main: exiting\n");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  aio_read_util(argc - 1, argv + 1);

  return 0;
}

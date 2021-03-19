set(CORO_SORT_SOURCES main.c coroutine.c sort.c aio_read.c)

add_executable(coro-sort ${CORO_SORT_SOURCES})

set(CORO_SORT_SOURCES main.c coroutine.c sort.c aio_read_util.c coro_spawn_to_sort.c)

add_executable(coro-sort ${CORO_SORT_SOURCES})
target_link_libraries(coro-sort rt)

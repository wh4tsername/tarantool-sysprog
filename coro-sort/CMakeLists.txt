set(CORO_SORT_SOURCES main.c coroutine.c sort.c aio_read_util.c scheduler.c merge_sorted_arrays.c)

add_executable(coro-sort ${CORO_SORT_SOURCES})
target_link_libraries(coro-sort rt)

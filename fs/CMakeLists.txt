set(FS_SOURCES userfs.c file_descriptors_table.c file.c block.c)

add_executable(fs test.c ${FS_SOURCES})

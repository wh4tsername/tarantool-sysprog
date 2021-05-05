CC	=  cc

BASE_SOURCES    = test.c
SOURCES		= $(BASE_SOURCES) userfs.c file_descriptors_table.c file.c block.c
OBJS		= $(SOURCES:.c=.o)
EXECUTABLE	= fs

all: test

build: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $(OBJS) -o $@

.c.o:
	$(CC) -c $< -o $@

test: build
	./$(EXECUTABLE)

clean:
	rm -rf $(EXECUTABLE) $(OBJS)

CFLAGS = -Wall -lm
PPOS_OBJECTS = ppos_core.o queue.o circular_buffer.o
TEST_OBJECTS = $(PPOS_OBJECTS) test.o

all: test

debug: CFLAGS += -DDEBUG -g 
debug: test

test: $(TEST_OBJECTS)
	gcc $(TEST_OBJECTS) -o test $(CFLAGS)

test.o: test.c
	gcc -c test.c -o test.o $(CFLAGS)

ppos_core.o: ppos_core.c ppos_data.h ppos.h
	gcc -c ppos_core.c $(CFLAGS)

queue.o: queue.c queue.h
	gcc -c queue.c -o queue.o $(CFLAGS)

circular_buffer.o: circular_buffer.c circular_buffer.h
	gcc -c circular_buffer.c -o circular_buffer.o $(CFLAGS)

clean: 
		rm -f *.o

purge:  clean
		rm -f test
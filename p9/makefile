CFLAGS = -Wall
PPOS_OBJECTS = ppos_core.o queue.o
TEST_OBJECTS = $(PPOS_OBJECTS) test.o

all: test

debug: CFLAGS += -DDEBUG -g 
debug: test

test: $(TEST_OBJECTS)
	gcc $(TEST_OBJECTS) -o test $(CFLAGS)

test1.o: test1.c
	gcc -c test.c -o test.o $(CFLAGS)

ppos_core.o: ppos_core.c ppos_data.h ppos.h
	gcc -c ppos_core.c $(CFLAGS)

queue.o: queue.c queue.h
	gcc -c queue.c -o queue.o $(CFLAGS)

clean: 
		rm -f *.o

purge:  clean
		rm -f test
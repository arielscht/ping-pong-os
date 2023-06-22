CFLAGS = -Wall -lm -lrt
PPOS_OBJECTS = ppos_core.o queue.o circular_buffer.o ppos_disk.o disk.o
TEST1_OBJECTS = $(PPOS_OBJECTS) test1.o
TEST2_OBJECTS = $(PPOS_OBJECTS) test2.o

all: test1 test2

debug: CFLAGS += -DDEBUG -g 
debug: test1 test2

test1: $(TEST1_OBJECTS)
	gcc $(TEST1_OBJECTS) -o test1 $(CFLAGS)

test2: $(TEST2_OBJECTS)
	gcc $(TEST2_OBJECTS) -o test2 $(CFLAGS)

test1.o: test1.c
	gcc -c test1.c -o test1.o $(CFLAGS)

test2.o: test2.c
	gcc -c test2.c -o test2.o $(CFLAGS)

ppos_core.o: ppos_core.c ppos_data.h ppos.h
	gcc -c ppos_core.c $(CFLAGS)

queue.o: queue.c queue.h
	gcc -c queue.c -o queue.o $(CFLAGS)

circular_buffer.o: circular_buffer.c circular_buffer.h
	gcc -c circular_buffer.c -o circular_buffer.o $(CFLAGS)

ppos_disk.o: ppos_disk.c ppos_disk.h
	gcc -c ppos_disk.c -o ppos_disk.o $(CFLAGS)

disk.o: disk.c disk.h
	gcc -c disk.c -o disk.o $(CFLAGS)

clean: 
		rm -f *.o

purge:  clean
		rm -f test1 test2
CFLAGS = -Wall
PPOS_OBJECTS = ppos_core.o queue.o
PRODCONS_OBJECTS = $(PPOS_OBJECTS) pingpong_prodcons.o circular_buffer.o

all: pingpong_prodcons

debug: CFLAGS += -DDEBUG -g 
debug: pingpong_prodcons

pingpong_prodcons: $(PRODCONS_OBJECTS)
	gcc $(PRODCONS_OBJECTS) -o pingpong_prodcons $(CFLAGS)

pingpong_prodcons.o: pingpong_prodcons.c
	gcc -c pingpong_prodcons.c -o pingpong_prodcons.o $(CFLAGS)

ppos_core.o: ppos_core.c ppos_data.h ppos.h
	gcc -c ppos_core.c $(CFLAGS)

queue.o: queue.c queue.h
	gcc -c queue.c -o queue.o $(CFLAGS)

circular_buffer.o: circular_buffer.c circular_buffer.h
	gcc -c circular_buffer.c -o circular_buffer.o $(CFLAGS)

clean: 
		rm -f *.o

purge:  clean
		rm -f pingpong_prodcons
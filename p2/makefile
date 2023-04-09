CFLAGS = -Wall
PPOS_OBJECTS = ppos_core.o
TASK1_OBJECTS = $(PPOS_OBJECTS) task1.o
TASK2_OBJECTS = $(PPOS_OBJECTS) task2.o
TASK3_OBJECTS = $(PPOS_OBJECTS) task3.o

all: task1 task2 task3

debug: CFLAGS += -DDEBUG -g 
debug: task1 task2 task3

task1: $(TASK1_OBJECTS)
	gcc $(TASK1_OBJECTS) -o task1 $(CFLAGS)

task2: $(TASK2_OBJECTS)
	gcc $(TASK2_OBJECTS) -o task2 $(CFLAGS)
	
task3: $(TASK3_OBJECTS)
	gcc $(TASK3_OBJECTS) -o task3 $(CFLAGS)

task1.o: pingpong-tasks1.c
	gcc -c pingpong-tasks1.c -o task1.o $(CFLAGS)

task2.o: pingpong-tasks2.c
	gcc -c pingpong-tasks2.c -o task2.o $(CFLAGS)
	
task3.o: pingpong-tasks3.c
	gcc -c pingpong-tasks3.c -o task3.o $(CFLAGS)

ppos_core.o: ppos_core.c ppos_data.h ppos.h
	gcc -c ppos_core.c $(CFLAGS)

clean: 
		rm -f *.o

purge:  clean
		rm -f task1 task2 task3
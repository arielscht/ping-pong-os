#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ppos.h"
#include "circular_buffer.h"

#define BUFFER_CAPACITY 5

semaphore_t s_buffer, s_items, s_slots;
task_t producer_task1, producer_task2, producer_task3, consumer_task1, consumer_task2;
buffer_t buffer;

int bounded_random(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

void producer(void *arg)
{
    int item;

    for (;;)
    {
        task_sleep(1000);
        item = bounded_random(0, 99);

        sem_down(&s_slots);
        sem_down(&s_buffer);
        buffer_add(&buffer, &item);
        sem_up(&s_buffer);
        sem_up(&s_items);

        printf("%s produced %d\n", (char *)arg, item);
    }
}

void consumer(void *arg)
{
    int item;
    for (;;)
    {
        sem_down(&s_items);
        sem_down(&s_buffer);
        buffer_remove(&buffer, &item);
        sem_up(&s_buffer);
        sem_up(&s_slots);

        printf("%s consumed %d\n", (char *)arg, item);
        task_sleep(1000);
    }
}

int main()
{
    ppos_init();
    srand(time(NULL));

    buffer_init(&buffer, BUFFER_CAPACITY, sizeof(int));

    sem_init(&s_buffer, BUFFER_CAPACITY);
    sem_init(&s_items, 0);
    sem_init(&s_slots, BUFFER_CAPACITY);

    task_init(&producer_task1, producer, "p1");
    task_init(&producer_task2, producer, "p2");
    task_init(&producer_task3, producer, "p3");
    task_init(&consumer_task1, consumer, "                  c1");
    task_init(&consumer_task2, consumer, "                  c2");

    task_exit(0);
}
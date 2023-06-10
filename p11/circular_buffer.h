#ifndef __CIRCULAR_BUFFER__
#define __CIRCULAR_BUFFER__

typedef struct buffer_t
{
    void *items;
    int head;
    int tail;
    int size;
    int item_size;
} buffer_t;

int buffer_init(buffer_t *buffer, int buffer_size, int item_size);

void buffer_add(buffer_t *buffer, void *item);

void buffer_remove(buffer_t *buffer, void *item);

int buffer_destroy(buffer_t *buffer);

#endif
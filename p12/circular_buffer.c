#include <stdlib.h>
#include "circular_buffer.h"

int buffer_init(buffer_t *buffer, int buffer_size, int item_size)
{
    buffer->items = calloc(buffer_size, item_size);
    if (buffer->items == NULL)
    {
        return -1;
    }
    buffer->head = 0;
    buffer->tail = -1;
    buffer->size = buffer_size;
    return 0;
};

void buffer_add(buffer_t *buffer, int item)
{
    buffer->tail = (buffer->tail + 1) % buffer->size;
    buffer->items[buffer->tail] = item;
}

int buffer_remove(buffer_t *buffer)
{
    int item = buffer->items[buffer->head];
    buffer->head = (buffer->head + 1) % buffer->size;
    return item;
}

int buffer_destroy(buffer_t *buffer)
{
    if (buffer->size == 0 || buffer->items == NULL)
    {
        return -1;
    }
    free(buffer->items);
    buffer->size = 0;
    return 0;
}
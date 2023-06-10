#include <stdlib.h>
#include <strings.h>
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
    buffer->item_size = item_size;
    return 0;
};

void buffer_add(buffer_t *buffer, void *item)
{
    buffer->tail = (buffer->tail + 1) % buffer->size;
    bcopy(item, &buffer->items[buffer->tail], buffer->item_size);
}

void buffer_remove(buffer_t *buffer, void *item)
{
    void *item_to_remove = &buffer->items[buffer->head];
    buffer->head = (buffer->head + 1) % buffer->size;
    bcopy(item_to_remove, item, buffer->item_size);
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
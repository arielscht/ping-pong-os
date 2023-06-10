typedef struct buffer_t
{
    int *items;
    int head;
    int tail;
    int size;
} buffer_t;

int buffer_init(buffer_t *buffer, int buffer_size, int item_size);

void buffer_add(buffer_t *buffer, int item);

int buffer_remove(buffer_t *buffer);

int buffer_destroy(buffer_t *buffer);
// GRR20203949 Ariel Evaldt Schmitt

#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

int queue_size(queue_t *queue)
{
    queue_t *cur_element = NULL;
    queue_t *initial_element = NULL;
    int size = 0;

    if (queue == NULL)
    {
        return 0;
    }

    initial_element = queue;
    cur_element = queue;

    do
    {
        size++;
        cur_element = cur_element->next;
    } while (cur_element != initial_element);

    return size;
}

void queue_print(char *name, queue_t *queue, void print_elem(void *))
{
    queue_t *initial_element;
    queue_t *cur_element;

    initial_element = queue;
    cur_element = queue;

    if (queue == NULL)
    {
        return;
    }

    printf("%s", name);
    do
    {
        print_elem(cur_element);
        printf(" ");
        cur_element = cur_element->next;
    } while (initial_element != cur_element);

    printf("\n");
}

int queue_append(queue_t **queue, queue_t *elem)
{
    if (queue == NULL)
    {
        fprintf(stderr, "The queue does not exist.\n");
        return -1;
    }
    if (elem == NULL)
    {
        fprintf(stderr, "The element does not exist.\n");
        return -2;
    }
    if (elem->prev != NULL || elem->next != NULL)
    {
        fprintf(stderr, "The element already is in a queue.\n");
        return -3;
    }

    if (*queue == NULL)
    {
        *queue = elem;
        elem->prev = elem;
        elem->next = elem;
        return 0;
    }
    else
    {
        (*queue)->prev->next = elem;
        elem->prev = (*queue)->prev;
        elem->next = *queue;
        (*queue)->prev = elem;
        return 0;
    }
}

int queue_remove(queue_t **queue, queue_t *elem)
{
    queue_t *cur_element = NULL;
    queue_t *initial_element = NULL;
    int belongs = 0;

    if (queue == NULL)
    {
        fprintf(stderr, "The queue does not exist.\n");
        return -1;
    }
    if (*queue == NULL)
    {
        fprintf(stderr, "The queue is empty.\n");
        return -2;
    }
    if (elem == NULL)
    {
        fprintf(stderr, "The element does not exist.\n");
        return -3;
    }

    initial_element = *queue;
    cur_element = *queue;
    do
    {
        if (elem == cur_element)
        {
            belongs = 1;
        }
        cur_element = cur_element->next;
    } while (initial_element != cur_element && belongs == 0);

    if (belongs == 0)
    {
        fprintf(stderr, "The element does not belong to the queue.\n");
        return -4;
    }

    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;

    if (*queue == elem)
    {
        if (elem->next == elem)
        {
            *queue = NULL;
        }
        else
        {
            *queue = elem->next;
        }
    }

    elem->next = NULL;
    elem->prev = NULL;

    return 0;
}
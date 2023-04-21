#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>
#include <string.h>

#include "ppos_data.h"
#include "ppos.h"
#include "queue.h"

#define STACKSIZE 64 * 1024
#define MAIN_ID 0

task_t main_task, dispatcher_task;
task_t *current_task = &main_task;
int incremental_id = 1;

queue_t *ready_queue;

void free_task_stack(task_t *task)
{
    if (task->context.uc_stack.ss_sp != NULL)
    {
        free(task->context.uc_stack.ss_sp);
    }
}

task_t *scheduler()
{
    task_t *next_task;

    if (queue_size(ready_queue) == 0)
    {
        return NULL;
    }

    next_task = (task_t *)ready_queue;
    ready_queue = ready_queue->next;
    return next_task;
}

void dispatcher()
{
    task_t *next_task;

    while (queue_size(ready_queue) > 0)
    {
        next_task = scheduler();

        if (next_task != NULL)
        {
            task_switch(next_task);

            switch (next_task->status)
            {
            case TERMINATED:
                queue_remove((queue_t **)&ready_queue, (queue_t *)next_task);
                free_task_stack(next_task);
                break;
            default:
                break;
            }
        }
    }

    task_exit(0);
}

void ppos_init()
{
    setvbuf(stdout, 0, _IONBF, 0);
    task_init(&dispatcher_task, dispatcher, NULL);
    main_task.id = MAIN_ID;
    main_task.status = READY;
    queue_append((queue_t **)&ready_queue, (queue_t *)&main_task);
    task_switch(&dispatcher_task);
}

int task_init(task_t *task, void (*start_routine)(void *), void *arg)
{
    char *stack;

    getcontext(&task->context);

    stack = malloc(STACKSIZE);
    if (stack)
    {
        task->context.uc_stack.ss_sp = stack;
        task->context.uc_stack.ss_size = STACKSIZE;
        task->context.uc_stack.ss_flags = 0;
        task->context.uc_link = 0;
    }
    else
    {
        perror("Error creating task stack\n");
        return 1;
    }

    makecontext(&task->context, (void *)start_routine, 1, arg);

    task->id = incremental_id;
    task->status = READY;
    incremental_id++;

#ifdef DEBUG
    printf("task_init: task %d initialized\n", task->id);
#endif
    if (task != &dispatcher_task)
    {
        queue_append((queue_t **)&ready_queue, (queue_t *)task);
    }
    return task->id;
}

int task_switch(task_t *task)
{
    task_t *aux;

    if (current_task == task)
    {
        perror("You can't switch to the same task\n");
        return -1;
    }

#ifdef DEBUG
    printf("task_switch: switching context %d -> %d\n", current_task->id, task->id);
#endif
    aux = current_task;
    current_task = task;
    swapcontext(&aux->context, &task->context);
    return 0;
}

void task_exit(int exit_code)
{
#ifdef DEBUG
    printf("task_exit: exiting task %d\n", current_task->id);
#endif

    if (current_task == &dispatcher_task)
    {
        free_task_stack(&dispatcher_task);
        exit(0);
    }
    else
    {
        current_task->status = TERMINATED;
        task_switch(&dispatcher_task);
    }
}

int task_id()
{
    return current_task->id;
}

void task_yield()
{
    task_switch(&dispatcher_task);
}
#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>

#include "ppos_data.h"
#include "ppos.h"

#define STACKSIZE 64 * 1024

task_t main_task;
task_t *current_task = &main_task;
int incremental_id = 1;

void ppos_init()
{
    main_task.id = 0;
    setvbuf(stdout, 0, _IONBF, 0);
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
    incremental_id++;

#ifdef DEBUG
    printf("task_init: task %d initialized\n", task->id);
#endif

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

    if (current_task == &main_task)
    {
        exit(0);
    }
    else
    {
        if (current_task->context.uc_stack.ss_sp != NULL)
        {
            free(current_task->context.uc_stack.ss_sp);
        }
        task_switch(&main_task);
    }
}

int task_id()
{
    return current_task->id;
}
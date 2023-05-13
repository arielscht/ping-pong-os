// GRR20203949 Ariel Evaldt Schmitt

#include <stdio.h>
#include <ucontext.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include "ppos_data.h"
#include "ppos.h"
#include "queue.h"

#define STACKSIZE 64 * 1024
#define MAIN_ID 0

task_t main_task, dispatcher_task;
task_t *current_task = &main_task;
int incremental_id = 1;

queue_t *ready_queue;

struct itimerval system_clock;
struct sigaction clock_action;

void free_task_stack(task_t *task)
{
    if (task->context.uc_stack.ss_sp != NULL)
    {
        free(task->context.uc_stack.ss_sp);
    }
}

void clock_handler()
{
    if (current_task && current_task->type == USER)
    {
        if (current_task->quantum == 0)
        {
            task_switch(&dispatcher_task);
        }
        else
        {
            current_task->quantum--;
        }
    }
}

void init_clock()
{
    clock_action.sa_handler = clock_handler;
    sigemptyset(&clock_action.sa_mask);
    clock_action.sa_flags = 0;
    if (sigaction(SIGALRM, &clock_action, 0) < 0)
    {
        perror("Error setting clock action: ");
        exit(1);
    }

    system_clock.it_value.tv_usec = 1000;
    system_clock.it_value.tv_sec = 0;
    system_clock.it_interval.tv_usec = 1000;
    system_clock.it_interval.tv_sec = 0;

    if (setitimer(ITIMER_REAL, &system_clock, 0) < 0)
    {
        perror("Error setting timer: ");
        exit(1);
    }
}

task_t *scheduler()
{
    task_t *cur_task;
    task_t *next_task;
    short most_prioritary = PRIO_UPPER_BOUND + 1;

    if (queue_size(ready_queue) == 0)
    {
        return NULL;
    }

    cur_task = (task_t *)ready_queue;
    do
    {
        if (cur_task->dynamic_prio < most_prioritary)
        {
            next_task = cur_task;
            most_prioritary = next_task->dynamic_prio;
        }
        if (cur_task->dynamic_prio > -20)
        {
            cur_task->dynamic_prio -= 1;
        }
        cur_task = cur_task->next;
    } while (cur_task != (task_t *)ready_queue);

    next_task->dynamic_prio = next_task->static_prio;
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
    init_clock();

    task_init(&dispatcher_task, dispatcher, NULL);
    dispatcher_task.type = SYSTEM;
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
    task->type = USER;
    task->status = READY;
    task->static_prio = 0;
    task->dynamic_prio = 0;
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
    task->quantum = TASK_QUANTUM;
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
#ifdef DEBUG
    printf("task_yield: task %d yielded the CPU\n", current_task->id);
#endif
    task_switch(&dispatcher_task);
}

void task_setprio(task_t *task, int prio)
{
    if (prio < PRIO_LOWER_BOUND || prio > PRIO_UPPER_BOUND)
    {
        fprintf(stderr, "task_setprio: invalid priority value for task %d. It should be from %d to %d.\n", task->id, PRIO_LOWER_BOUND, PRIO_UPPER_BOUND);
        return;
    }

#ifdef DEBUG
    printf("task_setprio: task %d priority set to %d\n", task->id, prio);
#endif

    task->static_prio = prio;
    task->dynamic_prio = prio;
}

int task_getprio(task_t *task)
{
    if (task != NULL)
    {
        return task->static_prio;
    }
    return current_task->static_prio;
}
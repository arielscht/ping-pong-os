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
int tasks_quantity = 1; // starts with 1 due to the main function
int drivers_quantity = 0;

task_t *ready_queue, *suspended_queue, *sleep_queue, *drivers_queue;

struct itimerval system_clock;
struct sigaction clock_action;
unsigned int system_time = 0;

void free_task_stack(task_t *task)
{
    if (task->context.uc_stack.ss_sp != NULL)
    {
        free(task->context.uc_stack.ss_sp);
    }
}

void resume_drivers()
{
    task_t *cur_element = drivers_queue;
    task_t *initial_element = drivers_queue;

    if (drivers_queue == NULL)
    {
        return;
    }

    do
    {
        task_resume(cur_element, &drivers_queue);
        cur_element = cur_element->next;
    } while (cur_element != initial_element);
}

void resume_waiting_tasks()
{
    task_t *cur_element = suspended_queue;
    task_t *initial_element = suspended_queue;

    if (suspended_queue == NULL)
    {
        return;
    }

    do
    {
        if (cur_element->wait_task == current_task)
        {
            task_resume(cur_element, &suspended_queue);
        }
        cur_element = cur_element->next;
    } while (cur_element != initial_element);
}

void resume_sleeping_tasks()
{
    task_t *cur_element = sleep_queue;
    task_t *initial_element = sleep_queue;
    task_t *next_element = NULL;

    if (queue_size((queue_t *)sleep_queue) == 0)
    {
        return;
    }

    do
    {
        next_element = cur_element->next;
        if (system_time >= cur_element->wake_time)
        {
            if (cur_element == initial_element)
            {
                initial_element = next_element;
            }
            task_resume(cur_element, &sleep_queue);
        }
        cur_element = next_element;
    } while (cur_element != initial_element);
}

void clock_handler()
{
    system_time += 1;
    if (current_task && current_task->type == USER)
    {
        if (current_task->quantum == 0)
        {
            task_yield();
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

    if (queue_size((queue_t *)ready_queue) == 0)
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

    while (tasks_quantity > 0)
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
        resume_sleeping_tasks();

        if (finish_drivers())
            resume_drivers();
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
    task_yield();
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
    task->activations = 0;
    task->start_time = system_time;
    task->last_cpu_time = 0;
    incremental_id++;

#ifdef DEBUG
    printf("task_init: task %d initialized\n", task->id);
#endif
    if (task != &dispatcher_task)
    {
        queue_append((queue_t **)&ready_queue, (queue_t *)task);
        tasks_quantity++;
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
    task->activations += 1;
    aux = current_task;
    if (aux->activations > 0)
    {
        aux->cpu_time += system_time - aux->last_cpu_time;
    }
    current_task = task;
    current_task->last_cpu_time = system_time;
    swapcontext(&aux->context, &task->context);
    return 0;
}

void task_exit(int exit_code)
{
#ifdef DEBUG
    printf("task_exit: exiting task %d\n", current_task->id);
#endif
    current_task->exit_code = exit_code;
    unsigned int task_life_time = system_time - current_task->start_time;
    printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", current_task->id, task_life_time, current_task->cpu_time, current_task->activations);
    if (current_task == &dispatcher_task)
    {
        free_task_stack(&dispatcher_task);
        exit(0);
    }
    else
    {
        current_task->status = TERMINATED;
        tasks_quantity--;
        resume_waiting_tasks();
        task_yield();
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

unsigned int systime()
{
    return system_time;
}

int task_wait(task_t *task)
{
    int exit_code = -1;

#ifdef DEBUG
    printf("task_wait: task %d waits for task %d\n", current_task->id, task->id);
#endif

    if (task == NULL)
    {
        perror("task_wait: task pointer is NULL");
    }
    else if (task->status != TERMINATED)
    {
        current_task->wait_task = task;
        task_suspend(&suspended_queue);
        exit_code = current_task->wait_task->exit_code;
        current_task->wait_task = NULL;
    }
    return exit_code;
}

void task_suspend(task_t **queue)
{
#ifdef DEBUG
    printf("task_suspend: task %d was suspended\n", current_task->id);
#endif
    task_t **task_queue = queue;
    if (!queue)
    {
#ifdef DEBUG
        perror("task_suspend: the suspended queue is NULL, so I'm using the suspended_queue\n");
#endif
        task_queue = &suspended_queue;
    }

    if (queue_remove((queue_t **)&ready_queue, (queue_t *)current_task) == 0)
    {
        if (task_queue == &sleep_queue)
        {
            current_task->status = SLEEPING;
        }
        else if (task_queue == &suspended_queue)
        {
            current_task->status = SUSPENDED;
        }
        else if (task_queue == &drivers_queue)
        {
            current_task->status = DRIVER_SUSPENDED;
        }
        else
        {
            current_task->status = LOCKED;
        }
        queue_append((queue_t **)task_queue, (queue_t *)current_task);
        task_yield();
    }
}

void task_resume(task_t *task, task_t **queue)
{
#ifdef DEBUG
    printf("task_resume: task %d was resumed\n", task->id);
#endif
    task_t **task_queue = queue;
    if (!queue)
    {
#ifdef DEBUG
        perror("task_resume: the queue is NULL, so I'm using the suspended_queue\n");
#endif
        task_queue = &suspended_queue;
    }
    if (queue_remove((queue_t **)task_queue, (queue_t *)task) == 0)
    {
        task->status = READY;
        queue_append((queue_t **)&ready_queue, (queue_t *)task);
    }
}

void task_sleep(int time)
{
    current_task->wake_time = system_time + time;
#ifdef DEBUG
    printf("task_sleep: task %d went to sleep %dms wake system time is %dms\n", current_task->id, time, current_task->wake_time);
#endif
    task_suspend(&sleep_queue);
}

int sem_init(semaphore_t *semaphore, int value)
{
    semaphore->slots = value;
    semaphore->destroyed = 0;
    semaphore->queue = NULL;
    return 0;
}

int sem_down(semaphore_t *semaphore)
{
    if (semaphore->destroyed)
    {
        return -1;
    }
    __sync_fetch_and_sub(&semaphore->slots, 1);
    if (semaphore->slots < 0)
    {
        task_suspend(&semaphore->queue);
    }
    return 0;
}

int sem_up(semaphore_t *semaphore)
{
    if (semaphore->destroyed)
    {
        return -1;
    }
    __sync_fetch_and_add(&semaphore->slots, 1);
    if (semaphore->slots <= 0)
    {
        if (semaphore->queue)
        {
            task_resume(semaphore->queue, &semaphore->queue);
        }
    }
    return 0;
}

int sem_destroy(semaphore_t *semaphore)
{
    task_t *initial_element = semaphore->queue;
    task_t *current_element = semaphore->queue;
    task_t *next_element;

    semaphore->destroyed = 1;
    if (queue_size((queue_t *)semaphore->queue) != 0)
    {
        do
        {
            next_element = current_element->next;
            task_resume(current_element, &semaphore->queue);
            current_element = next_element;
        } while (current_element != initial_element);
    }
    return 0;
}

int mqueue_is_destroyed(mqueue_t *queue)
{
    if (
        queue->buffer_semaphore.destroyed ||
        queue->items_semaphore.destroyed ||
        queue->slots_semaphore.destroyed)
    {
        return 1;
    }
    return 0;
}

int mqueue_init(mqueue_t *queue, int max_msgs, int msg_size)
{
    if (buffer_init(&queue->buffer, max_msgs, msg_size))
    {
        return -1;
    }
    sem_init(&queue->buffer_semaphore, 1);
    sem_init(&queue->items_semaphore, 0);
    sem_init(&queue->slots_semaphore, max_msgs);
    return 0;
}

int mqueue_send(mqueue_t *queue, void *msg)
{
    if (mqueue_is_destroyed(queue))
    {
        return -1;
    }

    if (sem_down(&queue->slots_semaphore) || sem_down(&queue->buffer_semaphore))
    {
        return -1;
    }
    buffer_add(&queue->buffer, msg);
    if (sem_up(&queue->buffer_semaphore) || sem_up(&queue->items_semaphore))
    {
        return -1;
    }
    return 0;
}

int mqueue_recv(mqueue_t *queue, void *msg)
{
    if (mqueue_is_destroyed(queue))
    {
        return -1;
    }

    if (sem_down(&queue->items_semaphore) || sem_down(&queue->buffer_semaphore))
    {
        return -1;
    }
    buffer_remove(&queue->buffer, msg);
    if (sem_up(&queue->buffer_semaphore) || sem_up(&queue->slots_semaphore))
    {
        return -1;
    }
    return 0;
}

int mqueue_destroy(mqueue_t *queue)
{
#ifdef DEBUG
    printf("mqueue_destroy: message queue destroy requested\n");
#endif
    if (mqueue_is_destroyed(queue))
    {
        return -1;
    }

    if (buffer_destroy(&queue->buffer))
    {
        return -1;
    }

    if (
        sem_destroy(&queue->buffer_semaphore) ||
        sem_destroy(&queue->slots_semaphore) ||
        sem_destroy(&queue->items_semaphore))
    {
        return -1;
    }
    return 0;
}

int mqueue_msgs(mqueue_t *queue)
{
    int items;
    if (mqueue_is_destroyed(queue))
    {
        return -1;
    }

    items = queue->items_semaphore.slots;
    if (items <= 0)
    {
        return 0;
    }

    return items;
}

int finish_drivers()
{
    if (tasks_quantity - drivers_quantity <= 0)
    {
        return 1;
    }
    return 0;
}
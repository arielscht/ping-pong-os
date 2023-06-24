// GRR20203949 Ariel Evaldt Schmitt
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include "disk.h"
#include "ppos_disk.h"
#include "ppos_data.h"
#include "ppos.h"
#include "queue.h"

task_t disk_task;
disk_t disk;
struct sigaction disk_action;

void disk_handler()
{
    disk.signal_received = 1;
    if (disk_task.status == SUSPENDED)
    {
        task_resume(&disk_task, NULL); // By default it will use the suspended_queue
    }
}

void init_disk_signal()
{
    disk_action.sa_handler = disk_handler;
    sigemptyset(&disk_action.sa_mask);
    disk_action.sa_flags = 0;
    if (sigaction(SIGUSR1, &disk_action, 0) < 0)
    {
        perror("Error setting clock action: ");
        exit(1);
    }
}

void disk_driver(void *args)
{
    disk_req_t *request;
    task_t *task;

    for (;;)
    {
        request = disk.req_queue;
        task = disk.queue;

        sem_down(&disk.semaphore);

        if (disk.signal_received)
        {
            if (queue_remove((queue_t **)&disk.req_queue, (queue_t *)request) == 0)
            {
                free(request);
                task_resume(task, &disk.queue);
            }
            disk.signal_received = 0;
        }

        if (disk_cmd(DISK_CMD_STATUS, 0, 0) == DISK_STATUS_IDLE && disk.queue != NULL)
        {
            switch (request->operation)
            {
            case READ:
                disk_cmd(DISK_CMD_READ, request->block, request->buffer);
                break;
            case WRITE:
                disk_cmd(DISK_CMD_WRITE, request->block, request->buffer);
                break;
            default:
                break;
            }
        }

        sem_up(&disk.semaphore);
        task_suspend(NULL); // By default it will use the suspended_queue
    }

    task_exit(0);
}

int disk_mgr_init(int *numBlocks, int *blockSize)
{
    int disk_size, disk_block_size;
    if (disk_cmd(DISK_CMD_INIT, 0, 0))
    {
        return -1;
    }

    disk_size = disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
    if (disk_size < 0)
    {
        return -1;
    }

    disk_block_size = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);
    if (disk_block_size < 0)
    {
        return -1;
    }

    *numBlocks = disk_size;
    *blockSize = disk_block_size;

    sem_init(&disk.semaphore, 1);
    init_disk_signal();

    task_init(&disk_task, disk_driver, NULL);
    disk_task.type = SYSTEM;

    return 0;
}

int disk_block_read(int block, void *buffer)
{
    disk_req_t *request;

    sem_down(&disk.semaphore);

    request = (disk_req_t *)malloc(sizeof(disk_req_t));
    if (request == NULL)
    {
        return -1;
    }
    request->block = block;
    request->buffer = buffer;
    request->operation = READ;

    if (queue_append((queue_t **)&disk.req_queue, (queue_t *)request))
    {
        return -1;
    }

    if (disk_task.status == SUSPENDED)
    {
        task_resume(&disk_task, NULL);
    }

    sem_up(&disk.semaphore);

    task_suspend(&disk.queue);

    return 0;
}

int disk_block_write(int block, void *buffer)
{
    disk_req_t *request;

    sem_down(&disk.semaphore);

    request = (disk_req_t *)malloc(sizeof(disk_req_t));
    if (request == NULL)
    {
        return -1;
    }
    request->block = block;
    request->buffer = buffer;
    request->operation = WRITE;

    if (queue_append((queue_t **)&disk.req_queue, (queue_t *)request))
    {
        return -1;
    }

    if (disk_task.status == SUSPENDED)
    {
        task_resume(&disk_task, NULL);
    }

    sem_up(&disk.semaphore);

    task_suspend(&disk.queue);

    return 0;
}

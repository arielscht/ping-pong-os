// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__

#include "ppos_data.h"

// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

typedef enum disk_operation
{
    READ = 0,
    WRITE = 1,
} disk_operation;

typedef struct disk_req_t
{
    struct disk_req_t *prev, *next;
    disk_operation operation;
    int block;
    void *buffer;
} disk_req_t;

// estrutura que representa um disco no sistema operacional
typedef struct
{
    disk_req_t *req_queue;
    task_t *queue;
    semaphore_t semaphore;
    int signal_received;
} disk_t;

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init(int *numBlocks, int *blockSize);

// leitura de um bloco, do disco para o buffer
int disk_block_read(int block, void *buffer);

// escrita de um bloco, do buffer para o disco
int disk_block_write(int block, void *buffer);

#endif
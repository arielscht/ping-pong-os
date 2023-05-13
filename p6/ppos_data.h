// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h> // biblioteca POSIX de trocas de contexto

#define PRIO_UPPER_BOUND 20
#define PRIO_LOWER_BOUND -20
#define TASK_QUANTUM 20

// enum do status da task
typedef enum task_status
{
    READY = 0,
    EXECUTING = 1,
    SUSPENDED = 2,
    TERMINATED = 3,
} task_status;

typedef enum task_owner
{
    SYSTEM = 0,
    USER = 1,
} task_owner;

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
    struct task_t *prev, *next; // ponteiros para usar em filas
    int id;                     // identificador da tarefa
    ucontext_t context;         // contexto armazenado da tarefa
    task_status status;         // pronta, rodando, suspensa, ...
    short static_prio;          // prioridade estática da task
    short dynamic_prio;         // prioridade dinâmica da task
    short quantum;              // contador de quantum da task
    task_owner type;            // tipo da tarefa
} task_t;

// estrutura que define um semáforo
typedef struct
{
    // preencher quando necessário
} semaphore_t;

// estrutura que define um mutex
typedef struct
{
    // preencher quando necessário
} mutex_t;

// estrutura que define uma barreira
typedef struct
{
    // preencher quando necessário
} barrier_t;

// estrutura que define uma fila de mensagens
typedef struct
{
    // preencher quando necessário
} mqueue_t;

#endif

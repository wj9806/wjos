#ifndef __SEM_H__
#define __SEM_H__

#include "tools/list.h"

typedef struct _sem_t
{
    int count;
    //等待队列
    list_t wait_list;
} sem_t;

void sem_init(sem_t * sem, int init_count);

//等信号 信号量减一
void sem_wait(sem_t * sem);

//发信号 信号量加一
void sem_notify(sem_t * sem);

int sem_count(sem_t * sem);
#endif
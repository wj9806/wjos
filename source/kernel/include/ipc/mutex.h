#ifndef __MUTEX_H__
#define __MUTEX_H__

#include "core/task.h"
#include "tools/list.h"

typedef struct _mutex_t
{
    //上锁次数
    int locked_count;
    //锁的持有者
    task_t * owner;
    //等待队列
    list_t wait_list;
} mutex_t;

void mutex_init(mutex_t * mutex);

void mutex_lock(mutex_t * mutex);

void mutex_unlock(mutex_t * mutex);

#endif
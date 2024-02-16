#ifndef __TASK_H__
#define __TASK_H__

#include "comm/types.h"
#include "cpu/cpu.h"

typedef struct _task_t
{
    tss_t tss;
} task_t;

//entry 入口地址
int task_init(task_t * task, uint32_t entry, uint32_t esp);
#endif
#ifndef __TASK_H__
#define __TASK_H__

#include "comm/types.h"
#include "cpu/cpu.h"

typedef struct _task_t
{
    //ESP寄存器指针
    uint32_t * stack;
    tss_t tss;
    int tss_sel;
} task_t;

//entry 入口地址
int task_init(task_t * task, uint32_t entry, uint32_t esp);
void task_switch_from_to(task_t* from, task_t* to);
#endif
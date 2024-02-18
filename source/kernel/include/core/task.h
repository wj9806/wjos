#ifndef __TASK_H__
#define __TASK_H__

#include "comm/types.h"
#include "cpu/cpu.h"
#include "tools/list.h"

#define TASK_NAME_SIZE                        32
//期望每个任务执行100ms放弃时间片
//因为时钟中断是10ms一次
//所以10次节拍后放弃时间片
#define TASK_TIME_SLICE_DEFAULT               (100/10)

typedef struct _task_t
{
    //ESP寄存器指针
    //uint32_t * stack;
    enum {
        TASK_CREATED,    //任务创建
        TASK_RUNNING,    //任务运行
        TASK_SLEEP,      //任务睡眠
        TASK_READY,      //任务就绪
        TASK_WAITTING,   //任务等待
    } state;

    int sleep_ticks;
    int time_ticks;
    int slice_ticks;

    char name[TASK_NAME_SIZE];

    node_t run_node;
    node_t all_node;

    tss_t tss;
    int tss_sel;
} task_t;

//entry 入口地址
int task_init(task_t * task, const char * name, uint32_t entry, uint32_t esp);
void task_switch_from_to(task_t* from, task_t* to);

//任务管理器
typedef struct _task_manager_t
{
    //当前运行的任务
    task_t * curr_task;
    //就绪队列
    list_t ready_list;
    //已创建的任务
    list_t task_list;
    //睡眠队列
    list_t sleep_list;

    task_t first_task;

    //空闲进程
    task_t idle_task;
} task_manager_t;

void task_manager_init(void);

void task_first_init(void);

task_t * task_first_task(void);

void task_set_ready(task_t * task);

void task_set_block(task_t * task);

void task_set_sleep(task_t * task, uint32_t ticks);
void task_set_wakeup(task_t * task);

task_t * task_current(void);

//当前任务主动放弃cpu时间片
int sys_sched_yeild(void);

//任务分配
void task_dispatch(void);

void task_time_tick(void);

//延时毫秒数
void sys_sleep(uint32_t ms);
#endif
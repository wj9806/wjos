#include "core/task.h"
#include "comm/types.h"
#include "comm/cpu_instr.h"
#include "tools/klib.h"
#include "os_cfg.h"
#include "cpu/cpu.h"
#include "tools/log.h"
#include "cpu/irq.h"
#include "cpu/mmu.h"
#include "core/memory.h"

static uint32_t idle_task_stack[IDLE_TASK_SIZE];
static task_manager_t task_manager;

static int tss_init(task_t * task, int flag, uint32_t entry, uint32_t esp)
{
    int tss_sel = gdt_alloc_desc();
    if (tss_sel < 0)
    {
        log_printf("alloc tss failed.");
        return -1;
    }
    segment_desc_set(tss_sel, (uint32_t)&task->tss, sizeof(tss_t),
        SEG_P_PRESENT | SEG_DPL_0 | SEG_TYPE_TSS
    );
    
    kernel_memset(&task->tss, 0, sizeof(tss_t));

    int code_sel, data_sel;
    if (flag & TASK_FLAGS_SYSTEM)
    {
       code_sel = KERNEL_SELECTOR_CS;
       data_sel = KERNEL_SELECTOR_DS;
    }
    else
    {
        code_sel = task_manager.app_code_sel | SEG_CPL_3;
        data_sel = task_manager.app_data_sel | SEG_CPL_3;
    }

    uint32_t kernel_stack = memory_alloc_page();
    if (kernel_stack == 0)
    {
        goto tss_init_failed;
    }
    
    task->tss.eip = entry;
    task->tss.esp = esp;
    task->tss.esp0 = kernel_stack + MEM_PAGE_SIZE;
    task->tss.ss = data_sel;
    task->tss.ss0 = KERNEL_SELECTOR_DS;
    task->tss.es = task->tss.ds = task->tss.fs = task->tss.gs = data_sel;
    task->tss.cs = code_sel;
    task->tss.eflags = EFLAGS_IF | EFLAGS_DEFAULT;

    uint32_t page_dir = memory_create_uvm();
    if (page_dir == 0)
    {
        gdt_free_sel(tss_sel);
        goto tss_init_failed;
    }
    //设置页表
    task->tss.cr3 = page_dir;

    task->tss_sel = tss_sel;
    return 0;
tss_init_failed:
    gdt_free_sel(tss_sel);
    if(kernel_stack) 
    {
        memory_free_page(kernel_stack);
    }
    return -1;
}

int task_init(task_t * task, const char * name, int flag, uint32_t entry, uint32_t esp)
{
    ASSERT(task != (task_t*)0);
    tss_init(task, flag, entry, esp);

    kernel_strncpy(task->name, name, TASK_NAME_SIZE);
    task->state = TASK_CREATED;
    task->sleep_ticks = 0;
    task->time_ticks = TASK_TIME_SLICE_DEFAULT;
    task->slice_ticks = task->time_ticks;
    node_init(&task->all_node);
    node_init(&task->run_node);
    node_init(&task->wait_node);
    
    irq_state_t state = irq_enter_protection();
    //每个task指针都不一样
    task->tid = (uint32_t)task;
    task_set_ready(task);
    list_insert_last(&task_manager.task_list, &task->all_node);
    irq_leave_protection(state);

    // uint32_t * pesp = (uint32_t*)esp;
    // if (pesp)
    // {
    //     *(--pesp) = entry;
    //     //设置四个寄存器
    //     *(--pesp) = 0;
    //     *(--pesp) = 0;
    //     *(--pesp) = 0;
    //     *(--pesp) = 0;
    //     task->stack = pesp;
    // }
    return 0;
}

void simple_switch(uint32_t ** from, uint32_t * to);

//任务切换
void task_switch_from_to(task_t* from, task_t* to)
{
    switch_to_tss(to->tss_sel);
    //simple_switch(&from->stack, to->stack);
}

//空闲进程
static void idle_task_entry(void)
{
    for (;;) {hlt();}
}

void task_manager_init(void)
{
    int sel = gdt_alloc_desc();
    segment_desc_set(sel, 0x000000000, 0xFFFFFFFF, 
        SEG_P_PRESENT | SEG_DPL_3 | SEG_S_NORMAL | SEG_TYPE_DATA | SEG_TYPE_RW | SEG_D);
    task_manager.app_data_sel = sel;

    sel = gdt_alloc_desc();
    segment_desc_set(sel, 0x000000000, 0xFFFFFFFF, 
        SEG_P_PRESENT | SEG_DPL_3 | SEG_S_NORMAL | SEG_TYPE_CODE | SEG_TYPE_RW | SEG_D);
    task_manager.app_code_sel = sel;


    task_manager.curr_task = (task_t *) 0;
    list_init(&task_manager.ready_list);
    list_init(&task_manager.task_list);
    list_init(&task_manager.sleep_list);

    task_init(&task_manager.idle_task, "idle-task", TASK_FLAGS_SYSTEM, 
        (uint32_t)idle_task_entry, (uint32_t) (idle_task_stack + IDLE_TASK_SIZE));
}

void task_first_init(void)
{
    extern uint8_t s_first_start[], e_first_task[];
    void first_task_entry (void);
    uint32_t first_start = (uint32_t) first_task_entry;

    uint32_t copy_size = (uint32_t)(e_first_task - s_first_start);
    uint32_t alloc_size = 10 * MEM_PAGE_SIZE;
    ASSERT(copy_size < alloc_size);

    task_init(&task_manager.first_task, "first_task", 0, first_start, first_start + alloc_size);
    write_tr(task_manager.first_task.tss_sel);
    task_manager.curr_task = &task_manager.first_task;

    mmu_set_page_dir(task_manager.first_task.tss.cr3);

    // 分配一页内存供代码存放使用，然后将代码复制过去
    memory_alloc_page_for(first_start, alloc_size, PTE_P | PTE_W | PTE_U);
    kernel_memcpy((void *)first_start, s_first_start, copy_size);
}

task_t * task_first_task(void)
{
    return &task_manager.first_task;
}

task_t * task_current(void)
{
    return task_manager.curr_task;
}

//获取下一个需要运行的任务
task_t * task_next_run(void)
{
    if (list_count(&task_manager.ready_list) == 0)
    {
        return &task_manager.idle_task;
    }
    
    node_t * task_node = list_first(&task_manager.ready_list);
    //获取任务管理器中第一个任务对应的task_t
    return list_node_parent(task_node, task_t, run_node);
}

void task_set_ready(task_t * task)
{
    if (task == &task_manager.idle_task)
    {
        return;
    }
    
    list_insert_last(&task_manager.ready_list, &task->run_node);
    task->state = TASK_READY;
}

void task_set_block(task_t * task)
{
    if (task == &task_manager.idle_task)
    {
        return;
    }
    list_remove(&task_manager.ready_list, &task->run_node);
}

void task_set_sleep(task_t * task, uint32_t ticks)
{
    if (ticks <= 0)
    {
        return;
    }
    task->sleep_ticks = ticks;
    task->state = TASK_SLEEP;
    list_insert_last(&task_manager.sleep_list, &task->run_node);
}

void task_set_wakeup(task_t * task)
{
    list_remove(&task_manager.sleep_list, &task->run_node);
}

int sys_sched_yeild(void)
{
    irq_state_t state = irq_enter_protection();
    if (list_count(&task_manager.ready_list) > 1)
    {
        //把当前运行的任务放到就绪队列中
        task_t * curr_task = task_current();
        task_set_block(curr_task);
        task_set_ready(curr_task);

        task_dispatch();
    }
    irq_leave_protection(state);
    return 0;
}

//分派任务
void task_dispatch(void)
{
    irq_state_t state = irq_enter_protection();
    task_t * to = task_next_run();
    if (to != task_manager.curr_task)
    {
        //获取当前运行的任务
        task_t * from = task_current();
        task_manager.curr_task = to;
        to->state = TASK_RUNNING;
        //切换到to去运行
        task_switch_from_to(from, to);
    }
    irq_leave_protection(state);
}

//计算任务运行时间片
void task_time_tick(void)
{
    task_t * curr_task = task_current();
    if (--curr_task->slice_ticks == 0)
    {
        curr_task->slice_ticks = curr_task->time_ticks;
        task_set_block(curr_task);
        task_set_ready(curr_task);
        //task_dispatch();
    }

    //扫描睡眠队列
    node_t * curr = list_first(&task_manager.sleep_list);
    while (curr)
    {
        node_t * next = list_node_next(curr);
        task_t * task = list_node_parent(curr, task_t, run_node);
        if (--task->sleep_ticks == 0)
        {
            task_set_wakeup(task);
            task_set_ready(task);
        }
        curr = next;
    }
    
    task_dispatch();
}

void sys_sleep(uint32_t ms)
{
    irq_state_t state = irq_enter_protection();
    task_set_block(task_manager.curr_task);

    //向上取整
    task_set_sleep(task_manager.curr_task, (ms + OS_TICKS_MS -1) /OS_TICKS_MS);

    task_dispatch();
    irq_leave_protection(state);
}

int sys_gettid(void)
{
    task_t * task = task_current();

    return task->tid;
}
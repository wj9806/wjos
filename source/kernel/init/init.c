#include "init.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/time.h"

#include "tools/log.h"
#include "tools/list.h"
#include "tools/klib.h"
#include "os_cfg.h"
#include "core/task.h"
#include "core/memory.h"

/**
 * 内核入口
 */
void kernel_init (boot_info_t * boot_info) 
{
    //cpu 初始化
    cpu_init();
    //添加缺省的异常捕获函数
    irq_init();
    log_init();
    
    memory_init(boot_info);

    time_init();
    task_manager_init();

}

void move_to_first_task(void)
{
    task_t * curr = task_current();
    ASSERT(curr != 0);

    tss_t * tss = &(curr->tss);
    __asm__ __volatile__(
        "push %[ss]\n\t"
        "push %[esp]\n\t"
        "push %[eflags]\n\t"
        "push %[cs]\n\t"
        "push %[eip]\n\t"
        "iret"::[ss]"r"(tss->ss), [esp]"r"(tss->esp), 
        [eflags]"r"(tss->eflags), [cs]"r"(tss->cs), [eip]"r"(tss->eip)
    );
}

void main_init (void)
{
    log_printf("wjos-kernel is running....");
    log_printf("wjos-kernel version: %s, date: %s", OS_VERSION, OS_TIME);
    
    task_first_init();
    move_to_first_task();
}
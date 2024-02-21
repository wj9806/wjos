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
    ASSERT(boot_info->ram_region_count != 0);
    //cpu 初始化
    cpu_init();
    memory_init(boot_info);
    log_init();
    //添加缺省的异常捕获函数
    irq_init();
    time_init();
    task_manager_init();
}

void move_to_first_task(void)
{
    task_t * curr = task_current();
    ASSERT(curr != 0);

    tss_t * tss = &(curr->tss);
    __asm__ __volatile__(
        "jmp *%[ip]"::[ip]"r"(tss->eip)
    );
}

void main_init (void)
{
    log_printf("wjos-kernel is running....");
    log_printf("wjos-kernel version: %s, date: %s", OS_VERSION, OS_TIME);
    
    task_first_init();
    move_to_first_task();
}
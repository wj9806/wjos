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

/**
 * 内核入口
 */
void kernel_init (boot_info_t * boot_info) 
{
    ASSERT(boot_info->ram_region_count != 0);
    //cpu 初始化
    init_cpu();
    log_init();
    //添加缺省的异常捕获函数
    irq_init();
    time_init();
    task_manager_init();
}

static uint32_t init_task_stack[1024];
static task_t init_task;

void init_task_entry()
{
    int count = 0;
    for(;;)
    {
        log_printf("task: %d", count++);
        sys_sleep(500);
        //task_switch_from_to(&init_task, task_first_task());
        //sys_sched_yeild();
    }
}

void main_init (void)
{
    log_printf("wjos-kernel is running....");
    log_printf("wjos-kernel version: %s, date: %s", OS_VERSION, OS_TIME);
    //int a = 3/0;
    

    task_init(&init_task, "init task", (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]);
    task_first_init();
    
    irq_enable_global();
    int count = 0;
    for(;;)
    {
        log_printf("main: %d", count++);
        sys_sleep(1000);
        //task_switch_from_to(task_first_task(), &init_task);
        //sys_sched_yeild();
    }
}
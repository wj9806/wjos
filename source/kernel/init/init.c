#include "init.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/time.h"
#include "tools/log.h"
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
}

static task_t first_task;
static uint32_t init_task_stack[1024];
static task_t init_task;

void init_task_entry()
{
    int count = 0;
    for(;;)
    {
        log_printf("task: %d", count++);
    }
}

void main_init (void)
{
    log_printf("wjos-kernel is running....");
    log_printf("wjos-kernel version: %s, date: %s", OS_VERSION, OS_TIME);
    //int a = 3/0;
    //irq_enable_global();

    task_init(&init_task, (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]);
    task_init(&first_task, 0, 0);

    int count = 0;
    for(;;)
    {
        log_printf("main: %d", count++);
    }
}
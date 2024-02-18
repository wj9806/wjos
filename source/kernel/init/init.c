#include "init.h"
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
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
        task_switch_from_to(&init_task, &first_task);
    }
}

void list_test()
{
    list_t list;
    list_init(&list);   
    node_t nodes[5];
    for(int i =0; i< 5; i++)
    {
        node_t * node = nodes + i;
        log_printf("node : 0x%x", node);
        list_insert_first(&list, node);
    }

    log_printf("list: first=0x%x, last=0x%x, count=%d", list_first(&list), 
        list_last(&list), list_count(&list));

    for(int i =0; i< 5; i++)
    {
        node_t * node = list_remove_last(&list);
        log_printf("node : 0x%x", node);
    }

    log_printf("list: first=0x%x, last=0x%x, count=%d", list_first(&list), 
        list_last(&list), list_count(&list));

    struct t_t
    {
        int i;
        node_t node;
    } v = {0x123456};
    node_t * t_node = &v.node;
    struct t_t * p = list_node_parent(t_node, struct t_t, node);
    ASSERT(p->i == 0x123456);
}

void main_init (void)
{
    list_test();

    log_printf("wjos-kernel is running....");
    log_printf("wjos-kernel version: %s, date: %s", OS_VERSION, OS_TIME);
    //int a = 3/0;
    //irq_enable_global();

    task_init(&init_task, (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]);
    task_init(&first_task, 0, 0);
    write_tr(first_task.tss_sel);
    
    int count = 0;
    for(;;)
    {
        log_printf("main: %d", count++);
        task_switch_from_to(&first_task, &init_task);
    }
}
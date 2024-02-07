#include "init.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"

/**
 * 内核入口
 */
void kernel_init (boot_info_t * boot_info) 
{
    //cpu 初始化
    init_cpu();
    //添加缺省的异常捕获函数
    irq_init();
}

void main_init (void)
{
    int a = 3/0;
    for(;;)
    {
        
    }
}
#include "init.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"

/**
 * 内核入口
 */
void kernel_init (boot_info_t * boot_info) 
{
    init_cpu();
    irq_init();
}

void main_init (void)
{
    //int a = 3/0;
    for(;;)
    {
        
    }
}
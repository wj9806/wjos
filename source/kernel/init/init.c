#include "init.h"
#include "comm/boot_info.h"
#include "cpu/cpu.h"

/**
 * 内核入口
 */
void kernel_init (boot_info_t * boot_info) 
{
    init_cpu();
    
    for (;;) {}
}
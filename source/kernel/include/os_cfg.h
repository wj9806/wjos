#ifndef __OS_CFG_H__
#define __OS_CFG_H__

#define GDT_TABLE_SIZE 256          // GDT表项数量
#define KERNEL_SELECTOR_CS (1 * 8) // 内核代码段描述符
#define KERNEL_SELECTOR_DS (2 * 8) // 内核数据段描述符
#define KERNEL_STACK_SIZE  (8 * 1024) // 内核栈

//10毫秒产生一次时钟中断
#define OS_TICKS_MS 10

#endif
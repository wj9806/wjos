/**
 * 自己动手写操作系统
 *
 * 系统引导部分，启动时由硬件加载运行，然后完成对二级引导程序loader的加载
 * boot扇区容量较小，仅512字节。由于dbr占用了不少字节，导致其没有多少空间放代码，
 * 所以功能只能最简化,并且要开启最大的优化-os
 */
__asm__(".code16gcc");

#include "boot.h"

//宏定义loader起始地址
#define LOADER_START_ADDR 0x8000

/**
 * Boot的C入口函数
 * 只完成一项功能，即从磁盘找到loader文件然后加载到内容中，并跳转过去
 */
void boot_entry(void) {
    //通过函数指针的方式强制转化该地址并调用该地址完成工程之间的跳转
    //跳转到loader/start.S
    ((void (*)(void))LOADER_START_ADDR)();
} 


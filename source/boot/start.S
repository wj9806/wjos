/**
 * 自己动手写操作系统
 *
 * 系统引导部分，启动时由硬件加载运行，然后完成对二级引导程序loader的加载
 * 该部分程序存储于磁盘的第1个扇区，在计算机启动时将会由BIOS加载到0x7c00处
 * 之后，将由BIOS跳转至0x7c00处开始运行
 * 
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
	#include "boot.h"

  	// 16位代码，务必加上
  	.code16
 	.text
	.global _start
	
	# 声明函数为外部声明的函数
	.extern boot_entry
_start:

	# 首先将段地址寄存器全部设置为0
	mov $0,%ax
	mov %ax,%ds
	mov %ax,%ss
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs

	# 初始化栈空间，设置栈的起始地址
	# 也可以使用  $_start,因为这个start程序汇编出来后就是放在0x7c00
	mov $0x7c00,%esp
	
	# BIOS会提供中断API
	# 通过BIOS软中断显示一个字符
	mov $0xe,%ah
	mov $'L',%al
	int $0x10

	# 利用BIOS软中断开始读取磁盘加载loader
	# INT13为磁盘读取的服务中断
	# 使用方法如下：
	# AH=02H
	# AL=扇区数
	# CH=柱面
	# DH=磁头
	# DL=驱动器，00H-7Fh：软盘；80H-0FFH：硬盘
	# ES：BX=缓冲区的地址
	# 出口参数：CF=0---操作成功，AH=00H,AL=传输的扇区数，否则AH=状态代码，参见功能01H中的说明

read_loader:
	mov $0x8000, %bx
	mov $0x2, %ah
	# cx=ch:cl各八位
	# 扇区设置为2是因为BIOS认为磁盘的扇区是以1开始编号
	mov $0x2, %cx
	# 64个扇区表示32KB,因为一个扇区512字节
	mov $64, %al
	mov $0x0080, %dx
	# 开始执行软中断
	int $0x13
	jc read_loader 

	# 当使用-exec x /20xb 0x8000指令读取缓冲区数据时会发现数据与0x200位置数据一样，为什么一样呢?
	# 因为我们是从硬盘的第2个扇区开始读，而第一个扇区为512字节，也就是表示16进制的0x200，因为是从0x0开始编号，因此第一扇区的结束位置为0x199
	# 所以读取到的数据是0x200开始往后的64*512个字节

	# 通过jmp指令直接跳转到函数中
	jmp boot_entry

	# 原地跳转指令
	jmp .
	
	// 引导结束段
	.section boot_end, "ax"
boot_sig: .byte 0x55, 0xaa

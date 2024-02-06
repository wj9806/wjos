#include "os_cfg.h"
    /*
     *gcc编译支持在汇编文件中引用c的头文件
    */
    .text
    .global _start
    .extern kernel_init
    .extern main_init
_start:
    # push %ebp
    # mov %esp, %ebp
    # mov 0x8(%ebp), %eax

    mov 4(%esp), %eax

    # kernel_init(boot_info_t)
    push %eax
    call kernel_init

    jmp $KERNEL_SELECTOR_CS, $gdt_reload
gdt_reload:
    mov $KERNEL_SELECTOR_DS, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	mov $(stack + KERNEL_STACK_SIZE), %esp

	jmp main_init

	.bss
.comm stack, KERNEL_STACK_SIZE    
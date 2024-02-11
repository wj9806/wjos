#include "comm/cpu_instr.h"
#include "cpu/irq.h"
#include "cpu/cpu.h"
#include "os_cfg.h"
#include "tools/log.h"

#define IDT_TABLE_NR 128

static gate_desc_t idt_table[IDT_TABLE_NR];

static void dump_core_regs (exception_frame_t * frame) {
    // 打印CPU寄存器相关内容
    log_printf("IRQ: %d, error code: %d.", frame->num, frame->error_code);
    log_printf("CS: %d\r\nDS: %d\r\nES: %d\r\nSS: %d\r\nFS:%d\r\nGS:%d",
               frame->cs, frame->ds, frame->es, frame->ds, frame->fs, frame->gs
    );
    log_printf("EAX:0x%x\r\n"
                "EBX:0x%x\r\n"
                "ECX:0x%x\r\n"
                "EDX:0x%x\r\n"
                "EDI:0x%x\r\n"
                "ESI:0x%x\r\n"
                "EBP:0x%x\r\n"
                "ESP:0x%x\r\n",
               frame->eax, frame->ebx, frame->ecx, frame->edx,
               frame->edi, frame->esi, frame->ebp, frame->esp);
    log_printf("EIP:0x%x\r\nEFLAGS:0x%x\r\n", frame->eip, frame->eflags);
}

static void do_default_handle(exception_frame_t * frame, const char* msg)
{
    log_printf("[ERROR] - IRQ Exception happend: %s ", msg);
    dump_core_regs(frame);
    for(;;)
    {
        hlt();
    }
}

void do_handle_unknown (exception_frame_t * frame)
{
    do_default_handle(frame, "unknow exeption");
}

void do_handle_divide (exception_frame_t * frame)
{
    do_default_handle(frame, "divide exeption");
}

void do_handle_Debug(exception_frame_t * frame) {
	do_default_handle(frame, "Debug Exception");
}

void do_handle_NMI(exception_frame_t * frame) {
	do_default_handle(frame, "NMI Interrupt.");
}

void do_handle_breakpoint(exception_frame_t * frame) {
	do_default_handle(frame, "Breakpoint.");
}

void do_handle_overflow(exception_frame_t * frame) {
	do_default_handle(frame, "Overflow.");
}

void do_handle_bound_range(exception_frame_t * frame) {
	do_default_handle(frame, "BOUND Range Exceeded.");
}

void do_handle_invalid_opcode(exception_frame_t * frame) {
	do_default_handle(frame, "Invalid Opcode.");
}

void do_handle_device_unavailable(exception_frame_t * frame) {
	do_default_handle(frame, "Device Not Available.");
}

void do_handle_double_fault(exception_frame_t * frame) {
	do_default_handle(frame, "Double Fault.");
}

void do_handle_invalid_tss(exception_frame_t * frame) {
	do_default_handle(frame, "Invalid TSS");
}

void do_handle_segment_not_present(exception_frame_t * frame) {
	do_default_handle(frame, "Segment Not Present.");
}

void do_handle_stack_segment_fault(exception_frame_t * frame) {
	do_default_handle(frame, "Stack-Segment Fault.");
}

void do_handle_general_protection(exception_frame_t * frame) {
	do_default_handle(frame, "General Protection.");
}

void do_handle_page_fault(exception_frame_t * frame) {
	do_default_handle(frame, "Page Fault.");
}

void do_handle_fpu_error(exception_frame_t * frame) {
	do_default_handle(frame, "X87 FPU Floating Point Error.");
}

void do_handle_alignment_check(exception_frame_t * frame) {
	do_default_handle(frame, "Alignment Check.");
}

void do_handle_machine_check(exception_frame_t * frame) {
	do_default_handle(frame, "Machine Check.");
}

void do_handle_smd_exception(exception_frame_t * frame) {
	do_default_handle(frame, "SIMD Floating Point Exception.");
}

void do_handle_virtual_exception(exception_frame_t * frame) {
	do_default_handle(frame, "Virtualization Exception.");
}

void do_handle_control_exception(exception_frame_t * frame) {
	do_default_handle(frame, "Control Exception.");
}

static void pic_init (void)
{
    // 边缘触发，级联，需要配置icw4, 8086模式
    outb(PIC0_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);
    // 对应的中断号起始序号0x20
    outb(PIC0_ICW2, IRQ_PIC_START);
    // 主片IRQ2有从片
    outb(PIC0_ICW3, 1 << 2);
    // 普通全嵌套、非缓冲、非自动结束、8086模式
    outb(PIC0_ICW4, PIC_ICW4_8086);

    // 边缘触发，级联，需要配置icw4, 8086模式
    outb(PIC1_ICW1, PIC_ICW1_ICW4 | PIC_ICW1_ALWAYS_1);
    // 起始中断序号，要加上8
    outb(PIC1_ICW2, IRQ_PIC_START + 8);
    // 没有从片，连接到主片的IRQ2上
    outb(PIC1_ICW3, 2);
    // 普通全嵌套、非缓冲、非自动结束、8086模式
    outb(PIC1_ICW4, PIC_ICW4_8086);

    // 禁止所有中断, 允许从PIC1传来的中断
    outb(PIC0_IMR, 0xFF & ~(1 << 2));
    outb(PIC1_IMR, 0xFF);
} 

void irq_init (void)
{
    for (int i =0 ; i < IDT_TABLE_NR; i++)
    {
        //设置捕获异常函数
        gate_desc_set(idt_table + i, KERNEL_SELECTOR_CS, (uint32_t) exception_handler_unknown, 
            GATE_P_PRESENT | GATE_DPL_0 | GATE_TYPE_INT
        );
    }
    //设置除法异常函数
    irq_install(IRQ0_DE,  (irq_handle_t) exception_handler_divide);
    //设置一些其他的异常函数
    irq_install(IRQ1_DB,  (irq_handle_t) exception_handler_Debug);
	irq_install(IRQ2_NMI, (irq_handle_t) exception_handler_NMI);
	irq_install(IRQ3_BP,  (irq_handle_t) exception_handler_breakpoint);
	irq_install(IRQ4_OF,  (irq_handle_t) exception_handler_overflow);
	irq_install(IRQ5_BR,  (irq_handle_t) exception_handler_bound_range);
	irq_install(IRQ6_UD,  (irq_handle_t) exception_handler_invalid_opcode);
	irq_install(IRQ7_NM,  (irq_handle_t) exception_handler_device_unavailable);
	irq_install(IRQ8_DF,  (irq_handle_t) exception_handler_double_fault);
	irq_install(IRQ10_TS, (irq_handle_t) exception_handler_invalid_tss);
	irq_install(IRQ11_NP, (irq_handle_t) exception_handler_segment_not_present);
	irq_install(IRQ12_SS, (irq_handle_t) exception_handler_stack_segment_fault);
	irq_install(IRQ13_GP, (irq_handle_t) exception_handler_general_protection);
	irq_install(IRQ14_PF, (irq_handle_t) exception_handler_page_fault);
	irq_install(IRQ16_MF, (irq_handle_t) exception_handler_fpu_error);
	irq_install(IRQ17_AC, (irq_handle_t) exception_handler_alignment_check);
	irq_install(IRQ18_MC, (irq_handle_t) exception_handler_machine_check);
	irq_install(IRQ19_XM, (irq_handle_t) exception_handler_smd_exception);
	irq_install(IRQ20_VE, (irq_handle_t) exception_handler_virtual_exception);
    irq_install(IRQ21_CP, (irq_handle_t) exception_handler_control_exception);

    lidt((uint32_t)idt_table, sizeof(idt_table));
    // 初始化pic 控制器
    pic_init();
}

int irq_install (int irq_num, irq_handle_t handle) 
{
    if (irq_num >= IDT_TABLE_NR)
    {
        return -1;
    }
    gate_desc_set(idt_table + irq_num, KERNEL_SELECTOR_CS, (uint32_t)handle,
        GATE_P_PRESENT | GATE_DPL_0 | GATE_TYPE_INT
    );
    return 0;
}

void irq_enable (int irq_num)
{
    if (irq_num < IRQ_PIC_START)
    {
        return;
    }
    irq_num -= IRQ_PIC_START;

    if (irq_num < 8)
    {
        uint8_t mask = inb(PIC0_IMR) & ~(1 << irq_num);
        outb(PIC0_IMR, mask);
    }
    else
    {
        irq_num -= 8;
        uint8_t mask = inb(PIC1_IMR) & ~(1 << irq_num);
        outb(PIC1_IMR, mask);
    }
}

void irq_disable (int irq_num)
{
    if (irq_num < IRQ_PIC_START)
    {
        return;
    }
    irq_num -= IRQ_PIC_START;

    if (irq_num < 8)
    {
        uint8_t mask = inb(PIC0_IMR) | (1 << irq_num);
        outb(PIC0_IMR, mask);
    }
    else
    {
        irq_num -= 8;
        uint8_t mask = inb(PIC1_IMR) | (1 << irq_num);
        outb(PIC1_IMR, mask);
    }
}

void irq_disable_global (void)
{
    cli();
}

void irq_enable_global (void)
{
    sti();
}

void pic_send_eoi(int irq_num)
{
    irq_num -= IRQ_PIC_START;

    // 从片也可能需要发送EOI
    if (irq_num >= 8) {
        outb(PIC1_OCW2, PIC_OCW2_EOI);
    }

    outb(PIC0_OCW2, PIC_OCW2_EOI);
}
#include "comm/cpu_instr.h"
#include "cpu/irq.h"
#include "cpu/cpu.h"
#include "os_cfg.h"

#define IDT_TABLE_NR 128

static gate_desc_t idt_table[IDT_TABLE_NR];

static void do_default_handle(exception_frame_t * frame, const char* msg)
{
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
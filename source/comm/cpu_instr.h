#ifndef __CPU_INSTR_H__
#define __CPU_INSTR_H__

#include "types.h"

//禁止中断发生
static inline void cli (void)
{
    __asm__ __volatile__("cli");
}

//允许中断发生
static inline void sti (void)
{
    __asm__ __volatile__("sti");
}

//inb指令
static inline uint8_t inb (uint16_t port)
{
    uint8_t rv;
    //inb al, dx
    __asm__ __volatile__("inb %[p],%[v]":[v]"=a"(rv) : [p]"d"(port));
    return rv;
}

//outb指令
static inline void outb(uint16_t port, uint8_t data)
{
    //outb al, dx
    __asm__ __volatile__("outb %[v],%[p]"::[p]"d"(port), [v]"a"(data));
}

static inline void lgdt (uint32_t start, uint32_t size)
{
    struct
    {
        uint16_t limit; //长度
        uint16_t start15_0; //起始地址的低16位
        uint16_t start31_16;//高16位
    } gdt;
    gdt.start31_16 = start >> 16;
    gdt.start15_0 = start & 0xFFFF;
    gdt.limit = size - 1;
    __asm__ __volatile__("lgdt %[g]"::[g]"m"(gdt));
}

static inline void lidt (uint32_t start, uint32_t size)
{
    struct
    {
        uint16_t limit; //长度
        uint16_t start15_0; //起始地址的低16位
        uint16_t start31_16;//高16位
    } idt;
    idt.start31_16 = start >> 16;
    idt.start15_0 = start & 0xFFFF;
    idt.limit = size - 1;
    __asm__ __volatile__("lidt %[g]"::[g]"m"(idt));
}


static inline uint32_t read_cr0(void)
{
    uint32_t cr0;

    __asm__ __volatile__("mov %%cr0, %[v]":[v]"=r"(cr0));
    return cr0;
}

static inline void write_cr0(uint32_t v)
{
    __asm__ __volatile__("mov %[v], %%cr0"::[v]"r"(v));
}

static inline void far_jump(uint32_t selector, uint32_t offset)
{
    uint32_t addr[] = {offset, selector};
    __asm__ __volatile__("ljmpl *(%[a])"::[a]"r"(addr));

}

//inw指令
static inline uint16_t inw (uint16_t port)
{
    uint16_t rv;
    //in ax, dx
    __asm__ __volatile__("in %[p],%[v]":[v]"=a"(rv) : [p]"d"(port));
    return rv;
}

#endif
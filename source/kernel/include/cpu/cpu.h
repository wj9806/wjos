#ifndef __CPU_H__
#define __CPU_H__

#include "comm/types.h"

#pragma pack(1)

typedef struct _segment_desc_t
{
    uint16_t limit15_0;
    uint16_t base15_0;
    uint8_t base23_16;
    uint16_t attr;
    uint8_t base31_24;
} segment_desc_t;

//调用门描述符
typedef struct _gate_desc_t
{
    uint16_t offset15_0;
    uint16_t selector;
    uint16_t attr;
    uint16_t offset31_16;
} gate_desc_t;

//task manager
typedef struct _tss_t
{
    uint32_t pre_link;
    uint32_t esp0, ss0, esp1, ss1, esp2, ss2;
    uint32_t cr3;
    uint32_t eip, eflags, eax, ecx, edx, ebx, esp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gx;
    uint32_t ldt;
    uint32_t iomap;
} tss_t;


#pragma pack()

#define SEG_G              (1 << 15)
#define SEG_D              (1 << 14)
#define SEG_P_PRESENT      (1 << 7)
//特权级
#define SEG_DPL_0          (0 << 5)
#define SEG_DPL_3          (3 << 5)

//系统段
#define SEG_S_SYSTEM       (0 << 4)
//普通
#define SEG_S_NORMAL       (1 << 4)

//代码段
#define SEG_TYPE_CODE      (1 << 3)
//数据段
#define SEG_TYPE_DATA      (0 << 3)
//可读写
#define SEG_TYPE_RW        (1 << 1)

// 中断32位门描述符
#define GATE_TYPE_INT	   (0xE << 8)		
//是否存在
#define GATE_P_PRESENT     (1 << 15)
#define GATE_DPL_0          (0 << 3)
//特权级0，最高特权级
#define GATE_DPL_3          (3 << 13)

void segment_desc_set (int selector, uint32_t base, uint32_t limit, uint16_t attr);

void init_cpu(void);

void gate_desc_set (gate_desc_t* desc, uint16_t selector, uint32_t offset, uint16_t attr);

#endif
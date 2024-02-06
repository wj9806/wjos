__asm__(".code16gcc");

#include "loader.h"

static boot_info_t boot_info;

static void show_msg(const char* msg)
{
    char c;
    while((c = *msg++) != '\0') 
    {
        //使用内联汇编方式打印字符串
        //内链汇编示例：https://wiki.osdev.org/Inline_Assembly/Examples
        __asm__ __volatile__ (
            "mov $0xe,%%ah\n\t"
            "mov %[ch],%%al\n\t"
            "int $0x10"::[ch]"r"(c)
        );
    }
}

static void detect_memory(void)
{
    show_msg("[wjos] - try to detect memory: ");

    uint32_t contID = 0;
    uint32_t signature, bytes;
    SMAP_entry_t smap_entry;

    boot_info.ram_region_count = 0;
    for (int i = 0; i < BOOT_RAM_REGION_MAX; i++)
    {
        SMAP_entry_t* entry = &smap_entry;
        __asm__ __volatile__ (
            "int $0x15" 
            : "=a"(signature), "=c"(bytes), "=b"(contID) 
            : "a"(0xE820), "b"(contID), "c"(24), "d"(0x534D4150), "D"(entry)
        );
        if (signature != 0x534D4150)
        {
            show_msg("failed.\r\n");
            return;
        }
        if (bytes > 20 && (entry->ACPI & 0x0001) == 0) 
            continue;
        //entry Type等于1时，说明这块内存区域可以被操作系统使用
        if (entry->Type == 1)
        {
            boot_info.ram_region_cfg[boot_info.ram_region_count].start = entry->BaseL;
            boot_info.ram_region_cfg[boot_info.ram_region_count].size = entry->LengthL;
            boot_info.ram_region_count++;
        }
        
        //读取完毕
        if (contID == 0)
            break;
    }
    show_msg("ok.\n\r");
}

uint16_t gdt_table[][4] = {
    {0, 0, 0, 0},
    {0xFFFF, 0x0000, 0x9a00, 0x00cf},
    {0xFFFF, 0x0000, 0x9200, 0x00cf},
};

/**
 * 从实模式切换到保护模式，需要遵循一定的流程
 * 1.禁用中断
 * 2.打开A20地址线
 * 3.加载GTD表
 * 4.设置CR0
 * 5.远跳转
 * 6.保护模式
*/
static void enter_protect_mode (void)
{
    cli();

    //打开A20地址线
    //in al, 0x92
    //or al, 2
    //out 0x92,al
    uint8_t v = inb(0x92);
    outb(0x92, v | 0x2);

    //加载gdt表
    lgdt((uint32_t) gdt_table, sizeof(gdt_table));

    //设置CR0 最低位置1
    uint32_t cr0 = read_cr0();
    write_cr0(cr0 | (1 << 0));

    //远跳转
    far_jump(8, (uint32_t) protect_mode_entry);
}

//实模式
//CPU上电复位后默认进入实模式，这种模式下没有保护机制，但提供了bios服务
/**
 * 1.只能访问1MB内存，内核寄存器最大为16位宽
 * 2.所有操作数最大为16位宽
 * 3.没有任何保护机制
 * 4.没有特权级支持
 * 5.没有分页机制和虚拟内存的支持
**/
//保护模式
/**
 * 寄存器位宽扩展至32位，最大访问可4GB内存
 * 所有操作数最大为32位宽，出入站也为32位
 * 提供4种特权级，操作系统可以运行在最高特权级，应用程序可运行在最低特权级
 * 支持虚拟内存 可开启分页机制
*/
void loader_entry (void)
{
    show_msg("[wjos] - loading os...\n\r");
    detect_memory();
    enter_protect_mode();
    for(;;)
    {
        
    }
}
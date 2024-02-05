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

//实模式
void loader_entry (void)
{
    show_msg("[wjos] - loading os...\n\r");
    detect_memory();
    for(;;)
    {
        
    }
}
#include "loader.h"

/**
 * sector 哪个扇区
*/
static void read_disk(int sector, int sector_count, uint8_t * buf)
{
    outb(0x1F6, 0xE0);
    outb(0x1F2, (uint8_t)(sector_count >> 8));
    outb(0x1F3, (uint8_t)(sector >> 24));
    outb(0x1F4, 0);
    outb(0x1F5, 0);

    outb(0x1F2, (uint8_t)(sector_count));
    outb(0x1F3, (uint8_t)(sector));
    outb(0x1F4, (uint8_t)(sector >> 8));
    outb(0x1F5, (uint8_t)(sector >> 16));

    outb(0x1F7, 0x24);

    uint16_t * data_buf = (uint16_t *) buf;
    while(sector_count --)
    {
        while ((inb(0x1F7) & 0x88) != 0x8) {}

        for (int i = 0; i < SECTOR_SIZE / 2; i++)
        {
            *data_buf++ = inw(0x1F0);
        }
    }
}

//保护模式下，bios功能无法使用，读取磁盘需要使用到LBA模式
void load_kernel(void)
{
    read_disk(100, 500, (uint8_t *)SYS_KERNEL_LOAD_ADDR);
    //跳转到kernel_init
    ((void (*)(void))SYS_KERNEL_LOAD_ADDR)();
    for(;;)
    {

    }
}
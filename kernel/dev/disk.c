#include "dev/disk.h"
#include "comm/cpu_instr.h"
#include "tools/log.h"
#include "tools/klib.h"

static disk_t disk_buf[DISK_CNT];

static void disk_send_cmd(disk_t * disk, uint32_t start_sector, uint32_t sector_count, int cmd)
{
    // 使用LBA寻址，并设置驱动器
    outb(DISK_DRIVE(disk), DISK_DRIVE_BASE | disk->drive);
    // 必须先写高字节
    outb(DISK_SECTOR_COUNT(disk), (uint8_t) (sector_count >> 8));
    // 扇区数高8位
    outb(DISK_LBA_LO(disk), (uint8_t) (start_sector >> 24));	
    // LBA参数的24~31位
    outb(DISK_LBA_MID(disk), 0);
    // 高于32位不支持
    outb(DISK_LBA_HI(disk), 0);	
    // 扇区数量低8位
    outb(DISK_SECTOR_COUNT(disk), (uint8_t) (sector_count));
    // LBA参数的0-7
    outb(DISK_LBA_LO(disk), (uint8_t) (start_sector >> 0));
    // LBA参数的8-15位
    outb(DISK_LBA_MID(disk), (uint8_t) (start_sector >> 8));
    // LBA参数的16-23位
    outb(DISK_LBA_HI(disk), (uint8_t) (start_sector >> 16));
    // 选择对应的主-从磁盘
	outb(DISK_CMD(disk), (uint8_t)cmd);
}

static void disk_read_datas (disk_t * disk, void * buf, int size)
{
    uint16_t * c = (uint16_t *) buf;
    for (int i = 0; i < size / 2; i++)
    {
        *c++ = inw(DISK_DATA(disk));
    }
}

static void disk_write_datas (disk_t * disk, void * buf, int size)
{
    uint16_t * c = (uint16_t *) buf;
    for (int i = 0; i < size / 2; i++)
    {
        outw(DISK_DATA(disk), *c++);
    }
}

static void print_disk_info(disk_t * disk)
{
    log_printf("%s:", disk->name);
    log_printf("  port_base: %x", disk->port_base);
    log_printf("  total_size: %d m", disk->sector_count * disk->sector_size / 1024 / 1024);
}

static int disk_wait_data (disk_t * disk)
{
    uint8_t status;
    do
    {
        status = inb(DISK_STATUS(disk));
        if ((status & (DISK_STATUS_BUSY | DISK_STATUS_DRQ | DISK_STATUS_ERR)) != DISK_STATUS_BUSY)
        {
            break;
        }
        
    } while (1);
    
    return (status & DISK_STATUS_ERR) ? -1 : 0;
}

static int identify_disk(disk_t * disk)
{
    disk_send_cmd(disk, 0, 0, DISK_CMD_IDENTIFY);
    int err = inb(DISK_STATUS(disk));
    if (err = 0)
    {
        log_printf("%s is not exist", disk->name);
        return -1;
    }
    err = disk_wait_data(disk);
    if (err < 0)
    {
        log_printf("disk:%s read failed", disk->name);
        return err;
    }
    uint16_t buf[256];
    disk_read_datas(disk, buf, sizeof(buf));
    disk->sector_count = *(uint32_t *) (buf + 100);
    disk->sector_size = SECTOR_SIZE;
    return err;
}

void disk_init(void)
{
    log_printf("disk start init..");

    kernel_memset(disk_buf, 0, sizeof(disk_buf));
    for (int i = 0; i < DISK_PER_CHANNEL; i++)
    {
        disk_t * disk = disk_buf + i;
        kernel_sprintf(disk->name, "sd%c", i + 'a');
        disk->drive = (i == 0) ? DISK_MASTER : DISK_SLAVE;
        disk->port_base = IOBASE_PRIMARY;

        int err = identify_disk(disk);
        if (err == 0)
        {
            print_disk_info(disk);
        }
        
    }
    
}
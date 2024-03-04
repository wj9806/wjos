#include "fs/fs.h"
#include "fs/file.h"
#include "comm/types.h"
#include "comm/elf.h"
#include "comm/cpu_instr.h"
#include "comm/boot_info.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "dev/console.h"
#include "dev/dev.h"
#include "core/task.h"

#define FS_TABLE_SIZE            8
#define TEMP_FILE_ID             100

static list_t mounted_list;
static fs_t fs_table[FS_TABLE_SIZE];
static list_t free_list;

extern fs_op_t devfs_op;

static uint8_t TEMP_ADDR[100*1024];
static uint8_t * temp_pos;


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

static int is_path_valid (const char * path)
{
    if ((path == (const char *)0) || (path[0] == '\0'))
    {
        return 0;
    }
    return 1;
}

int sys_open(const char * name, int flags, ...)
{
    if (kernel_strcmp(name, "tty", 3) == 0)
    {
        if (!is_path_valid(name))
        {
            log_printf("path is invalid");
            return -1;
        }

        int fd = -1;
        file_t * file = file_alloc();
        if (file)
        {
            fd = task_alloc_fd(file);
            if (fd < 0)
            {
                goto sys_open_failed;
            }
        }
        else
        {
            goto sys_open_failed;
        }
        if (kernel_strlen(name) < 5)
        {
            goto sys_open_failed;
        }
        
        int num = name[4] - '0';
        int dev_id = dev_open(DEV_TTY, num, 0);
        if (dev_id < 0)
        {
            goto sys_open_failed;
        }
        file->dev_id = dev_id;
        file->mode = 0;
        file->pos = 0;
        file->ref = 1;
        file->type = FILE_TTY;
        kernel_memcpy(file->file_name, (void *)name, FILE_NAME_SIZE);
        return fd;
sys_open_failed:
        if (file)
        {
            file_free(file);
        }
        if (fd >= 0)
        {
            task_remove_fd(fd);
        }
        return -1;
    } 
    else
    {
        if (name[0] == '/')
        {
            read_disk(5000, 80, (uint8_t *) TEMP_ADDR);
            temp_pos = (uint8_t *) TEMP_ADDR;
            return TEMP_FILE_ID;
        }
    }
    

    return -1;
}

int sys_read(int file, char * ptr, int len)
{
    if (file == TEMP_FILE_ID)
    {
        kernel_memcpy(ptr, temp_pos, len);
        temp_pos += len;
        return len;
    }
    else
    {
        //file = 0;

        file_t * p_file = task_file(file);
        if (!p_file)
        {
            log_printf("file not opened: %d", file);
            return -1;
        }
        
        return dev_read(p_file->dev_id, 0, ptr, len);
    }
}

int sys_write(int file, char * ptr, int len)
{
    file_t * p_file = task_file(file);
    if (!p_file)
    {
        log_printf("file not opened: %d", file);
        return -1;
    }
    
    return dev_write(p_file->dev_id, 0, ptr, len);
}

//ptr 相对于文件开头的指针
int sys_lseek(int file, int ptr, int dir)
{
    if (file == TEMP_FILE_ID)
    {
        temp_pos = (uint8_t *) (TEMP_ADDR + ptr);
        return 0;
    }
    return -1;
}

int sys_close(int file)
{
    return 0;
}

int sys_isatty(int file)
{
    return -1;
}

int sys_fstat(int file, struct stat * st)
{
    return -1;
}

static void mount_list_init(void)
{
    list_init(&free_list);
    for (int i = 0; i < FS_TABLE_SIZE; i++)
    {
        list_insert_first(&free_list, &fs_table[i].node);
    }
    list_init(&mounted_list);
}

static fs_op_t * get_fs_op(fs_type_t type, int major)
{
    switch (type)
    {
    case FS_DEVFS:
        return &(devfs_op);
    default:
        return (fs_op_t *)0;
    }
}

static fs_t * mount(fs_type_t type, char * mount_point, int dev_major, int dev_minor)
{
    fs_t * fs = (fs_t *)0;
    log_printf("start mount file system, name: %s, dev: %x", mount_point, dev_major);

    node_t * curr = list_first(&mounted_list);
    while (curr)
    {
        fs_t * fs = list_node_parent(curr, fs_t, node);
        //判断是否已经存在了
        if (kernel_strcmp(fs->mount_point, mount_point, FS_MOUNT_SIZE) == 0)
        {
            log_printf("fs already mounted");
            goto mount_failed;
        }
        curr = list_node_next(curr);
    }
    
    node_t * free_node = list_remove_first(&free_list);
    if (!free_node)
    {
        log_printf("no free fs, mount failed");
        goto mount_failed;
    }

    fs = list_node_parent(free_node, fs_t, node);
    fs_op_t * op = get_fs_op(type, dev_major);
    if (!op)
    {
        log_printf("unsupported fs type: %d", type);
        goto mount_failed;
    }
    kernel_memset(fs, 0, sizeof(fs_t));
    kernel_strncpy(fs->mount_point, mount_point, FS_MOUNT_SIZE);
    fs->op = op;
    if (fs->op->mount(fs, dev_major, dev_minor) < 0)
    {
        log_printf("fs mount failed, name: %s, dev: %x", mount_point, dev_major);
        goto mount_failed;
    }
    
    list_insert_last(&mounted_list, &fs->node);

    return fs;
mount_failed:
    if (fs)
    {
        list_insert_last(&free_list, &fs->node);
    }
    
    return (fs_t *) 0;
}

void fs_init(void)
{
    mount_list_init();
    file_table_init();

    fs_t * fs = mount(FS_DEVFS, "/dev", 0, 0);
    ASSERT(fs != (fs_t *)0);
}

int sys_dup(int file)
{
    if ((file < 0) || (file > TASK_OFILE_NR))
    {
        log_printf("file is invalid: %d", file);
        return -1;
    }
    file_t * p_file = task_file(file);
    if (!p_file)
    {
        log_printf("file not opened: %d", file);
        return -1;
    }
    
    int fd = task_alloc_fd(p_file);
    if (fd >= 0)
    {
        p_file->ref++;
        return fd;
    }
    log_printf("no task file avaliable");
    return -1;
}
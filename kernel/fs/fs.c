#include "fs/fs.h"
#include "fs/file.h"
#include "comm/types.h"
#include "comm/elf.h"
#include "comm/cpu_instr.h"
#include "comm/boot_info.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "dev/console.h"
#include "dev/disk.h"
#include "dev/dev.h"
#include "core/task.h"
#include <sys/file.h>

#define FS_TABLE_SIZE            8

static list_t mounted_list;
static fs_t fs_table[FS_TABLE_SIZE];
static list_t free_list;

extern fs_op_t devfs_op;
extern fs_op_t fatfs_op;

static fs_t * root_fs;

int path_to_num(const char * path, int * num)
{
    int n = 0;
    const char * c = path;
    while (*c)
    {
        n = n * 10 + *c - '0';
        c++;
    }
    *num = n;
    return 0;
}

const char * path_next_child(const char * path)
{
    // /dev/tty ==> tty
    const char * c = path;
    while (*c && (*c++ == '/')) {}
    while (*c && (*c++ != '/')) {}
    
    return *c ? c : (const char *)0;
}

int path_begin_with(const char * path, const char * str)
{
    const char * s1 = path, * s2 = str;
    while (*s1 && *s2 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *s2 == '\0';
}

static int is_fd_bed(int file)
{
    return (file < 0) || (file > TASK_OFILE_NR);
}

static int is_path_valid (const char * path)
{
    if ((path == (const char *)0) || (path[0] == '\0'))
    {
        return 0;
    }
    return 1;
}

static void fs_protect(fs_t * fs)
{
    if (fs->mutex)
    {
        mutex_lock(fs->mutex);
    }
}

static void fs_unprotect(fs_t * fs)
{
    if (fs->mutex)
    {
        mutex_unlock(fs->mutex);
    }
}

int sys_open(const char * name, int flags, ...)
{
    int fd = -1;
    file_t * file = file_alloc();
    if (!file)
    {
        return -1;
    }

    fd = task_alloc_fd(file);
    if (fd < 0)
    {
        goto sys_open_failed;
    }

    fs_t * fs = (fs_t *)0; 
    node_t * node = list_first(&mounted_list);
    while (node)
    {
        fs_t * curr = list_node_parent(node, fs_t, node);
        if (path_begin_with(name, curr->mount_point))
        {
            fs = curr;
            break;
        }
        node = list_node_next(node);
    }
    if (fs)
    {
        name = path_next_child(name);
    }
    else
    {
        fs = root_fs;
    }
    
    file->mode = flags;
    file->ref = 1;
    file->fs = fs;
    kernel_strncpy(file->file_name, name, FILE_NAME_SIZE);

    fs_protect(fs);
    int err = fs->op->open(fs, name, file);
    if (err < 0)
    {
        fs_unprotect(fs);
        //log_printf("open %s failed", name);
        goto sys_open_failed;
    }
    fs_unprotect(fs);
    return fd;
sys_open_failed:
    file_free(file);
    if (fd >= 0)
    {
        task_remove_fd(fd);
    }
    return -1;
}

int sys_read(int file, char * ptr, int len)
{
    if (is_fd_bed(file) || !ptr || !len)
    {
        return 0;
    }

    file_t * p_file = task_file(file);
    if (!p_file)
    {
        log_printf("file not opened: %d", file);
        return -1;
    }
    if (p_file->mode == O_WRONLY)
    {
        log_printf("file is write only: %d", file);
    }

    fs_t * fs = p_file->fs;
    fs_protect(fs);
    int res = fs->op->read(ptr, len, p_file);
    fs_unprotect(fs);
    return res;
}

int sys_write(int file, char * ptr, int len)
{
   if (is_fd_bed(file) || !ptr || !len)
    {
        return 0;
    }

    file_t * p_file = task_file(file);
    if (!p_file)
    {
        log_printf("file not opened: %d", file);
        return -1;
    }
    if (p_file->mode == O_RDONLY)
    {
        log_printf("file is read only: %d", file);
    }

    fs_t * fs = p_file->fs;
    fs_protect(fs);
    int res = fs->op->write(ptr, len, p_file);
    fs_unprotect(fs);
    return res;
}

//ptr 相对于文件开头的指针
int sys_lseek(int file, int ptr, int dir)
{
   if (is_fd_bed(file))
    {
        return 0;
    }

    file_t * p_file = task_file(file);
    if (!p_file)
    {
        log_printf("file not opened: %d", file);
        return -1;
    }

    fs_t * fs = p_file->fs;
    fs_protect(fs);
    int res = fs->op->seek(p_file, ptr, dir);
    fs_unprotect(fs);
    return res;
}

int sys_close(int file)
{
    if (is_fd_bed(file))
    {
        return 0;
    }
    file_t * p_file = task_file(file);
    if (!p_file)
    {
        log_printf("file not opened: %d", file);
        return -1;
    }
    ASSERT(p_file->ref > 0);
    fs_t * fs = p_file->fs;

    if (p_file->ref-- == 1)
    {
        fs_protect(fs);
        fs->op->close(p_file);
        fs_unprotect(fs);
        file_free(p_file);
    }
    
    task_remove_fd(file);
    return 0;
}

int sys_isatty(int file)
{
    if (is_fd_bed(file))
    {
        return 0;
    }
    file_t * p_file = task_file(file);
    if (!p_file)
    {
        log_printf("file not opened: %d", file);
        return -1;
    }
    return p_file->type == FILE_TTY;
}

int sys_ioctl(int fd, int cmd, int arg0, int arg1)
{
    if (is_fd_bed(fd))
    {
        log_printf("file is invalid: %d", fd);
        return -1;
    }
    file_t * p_file = task_file(fd);
    if (!p_file)
    {
        log_printf("file not opened: %d", fd);
        return -1;
    }
    
    fs_t * fs = p_file->fs;
    int err = -1;
    fs_protect(fs);
    if (fs->op->ioctl)
    {
        err = fs->op->ioctl(p_file, cmd, arg0, arg1);
    }
    fs_unprotect(fs);
    return err;
}

int sys_fstat(int file, struct stat * st)
{
    return -1;    if (is_fd_bed(file))
    {
        return 0;
    }
    file_t * p_file = task_file(file);
    if (!p_file)
    {
        log_printf("file not opened: %d", file);
        return -1;
    }
    kernel_memset(st, 0, sizeof(struct stat));
    fs_t * fs = p_file->fs;
    fs_protect(fs);
    int err = fs->op->stat(p_file, st);
    fs_unprotect(fs);
    return err;
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
    case FS_FATFS16:
        return &(fatfs_op);
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
    disk_init();
    
    fs_t * fs = mount(FS_DEVFS, "/dev", 0, 0);
    ASSERT(fs != (fs_t *)0);

    root_fs = mount(FS_FATFS16, "/home", ROOT_DEV);
    ASSERT(root_fs != (fs_t *)0);
}

int sys_dup(int file)
{
    if (is_fd_bed(file))
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
        file_inc_ref(p_file);
        return fd;
    }
    log_printf("no task file avaliable");
    return -1;
}

int sys_opendir(const char * name, DIR * dir)
{
    fs_protect(root_fs);
    int err = root_fs->op->opendir(root_fs, name, dir);
    fs_unprotect(root_fs);
    return err;
}

int sys_readdir(DIR * dir, struct dirent * dirent)
{
    fs_protect(root_fs);
    int err = root_fs->op->readdir(root_fs, dir, dirent);
    fs_unprotect(root_fs);
    return err;
}

int sys_closedir(DIR * dir)
{
    fs_protect(root_fs);
    int err = root_fs->op->closedir(root_fs, dir);
    fs_unprotect(root_fs);
    return err;
}

int sys_unlink(const char * path)
{
    fs_protect(root_fs);
    int err = root_fs->op->unlink(root_fs, path);
    fs_unprotect(root_fs);
    return err;
}
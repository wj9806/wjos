#include "fs/fatfs/fatfs.h"
#include "dev/dev.h"
#include "tools/log.h"
#include "tools/klib.h"
#include "core/memory.h"
#include "fs/fs.h"

int fatfs_mount(struct _fs_t * fs, int major, int minor)
{
    int dev_id = dev_open(major, minor, (void*)0);
    if (dev_id < 0)
    {
        log_printf("open disk failed. major: %x, minor: %x", major, minor);
        return -1;
    }
    
    dbr_t * dbr = (dbr_t *) memory_alloc_page();
    if (!dbr)
    {
        log_printf("fatfs mount failed. by alloc buf");
        goto mount_failed;
    }
    int cnt = dev_read(dev_id, 0, (char *) dbr, 1);
    if (cnt < 1)
    {
        log_printf("read dbr failed");
        goto mount_failed;
    }
    
    fat_t * fat = (fat_t *)&(fs->fat_data);
    fat->fat_buffer = (uint8_t *)dbr;
    fat->bytes_per_sec = dbr->BPB_BytsPerSec;
    fat->tbl_start = dbr->BPB_RsvdSecCnt;
    fat->tbl_sectors = dbr->BPB_FATSz16;
    fat->tbl_cnt = dbr->BPB_NumFATs;
    fat->root_ent_cnt = dbr->BPB_RootEntCnt;
    fat->sec_per_cluster = dbr->BPB_SecPerClus;
    fat->cluster_byte_size = fat->sec_per_cluster * dbr->BPB_BytsPerSec;
    fat->root_start = fat->tbl_start + fat->tbl_sectors * fat->tbl_cnt;
    fat->data_start = fat->root_start + fat->root_ent_cnt * 32 / SECTOR_SIZE;
    fat->fs = fs;
    fat->curr_sector = -1;
    if (fat->tbl_cnt != 2) {
        log_printf("fat table num error, major: %x, minor: %x", major, minor);
		goto mount_failed;
	}
    if (kernel_memcmp(dbr->BS_FileSysType, "FAT16", 5) != 0) {
        log_printf("not a fat16 file system, major: %x, minor: %x", major, minor);
        goto mount_failed;
    }
    // 记录相关的打开信息
    fs->type = FS_FATFS16;
    fs->data = &fs->fat_data;
    fs->dev_id = dev_id;
    return 0;
mount_failed:
    if (dbr) {
        memory_free_page((uint32_t)dbr);
    }
    dev_close(dev_id);
}

void fatfs_unmount(struct _fs_t * fs)
{
    fat_t * fat = (fat_t *)fs->data;

    dev_close(fs->dev_id);
    memory_free_page((uint32_t)fat->fat_buffer);
}

int fatfs_open(struct _fs_t * fs, const char * path, file_t * file)
{
    
    return -1;
}

int fatfs_read(char * buf, int size, file_t * file)
{
    return -1;
}

int fatfs_write(char * buf, int size, file_t * file)
{
    return -1;
}

void fatfs_close(file_t * file)
{

}

int fatfs_seek(file_t * file, uint32_t offset, int dir)
{
    return -1;
}

int fatfs_stat(file_t * file, struct stat * st)
{
    return -1;
}

int fatfs_opendir(struct _fs_t * fs, const char * name, DIR * dir)
{
    dir->index = 0;
    return 0;
}

void diritem_get_name (diritem_t * item, char * dest) {
    char * c = dest;
    char * ext = (char *)0;

    kernel_memset(dest, 0, SFN_LEN + 1);     // 最多11个字符
    for (int i = 0; i < 11; i++) {
        if (item->DIR_Name[i] != ' ') {
            *c++ = item->DIR_Name[i];
        }

        if (i == 7) {
            ext = c;
            *c++ = '.';
        }
    }

    // 没有扩展名的情况
    if (ext && (ext[1] == '\0')) {
        ext[0] = '\0';
    }
}

static int bread_sector (fat_t * fat, int sector) {
    if (sector == fat->curr_sector) {
        return 0;
    }

    int cnt = dev_read(fat->fs->dev_id, sector, fat->fat_buffer, 1);
    if (cnt == 1) {
        fat->curr_sector = sector;
        return 0;
    }
    return -1;
}

static diritem_t * read_dir_entry (fat_t * fat, int index) {
    if ((index < 0) || (index >= fat->root_ent_cnt)) {
        return (diritem_t *)0;
    }

    int offset = index * sizeof(diritem_t);
    int err = bread_sector(fat, fat->root_start + offset / fat->bytes_per_sec);
    if (err < 0) {
        return (diritem_t *)0;
    }
    return (diritem_t *)(fat->fat_buffer + offset % fat->bytes_per_sec);
}

file_type_t diritem_get_type (diritem_t * item) {
    if (item->DIR_Attr & (DIRITEM_ATTR_VOLUME_ID | DIRITEM_ATTR_HIDDEN | DIRITEM_ATTR_SYSTEM)) {
        return FILE_UNKNOWN;
    }
    if ((item->DIR_Attr & DIRITEM_ATTR_LONG_NAME) == DIRITEM_ATTR_LONG_NAME)
    {
        return FILE_UNKNOWN;
    }
    
    return item->DIR_Attr & DIRITEM_ATTR_DIRECTORY ? FILE_DIR : FILE_NORMAL;
}

int fatfs_readdir(struct _fs_t * fs, DIR * dir, struct dirent * dirent)
{
    fat_t * fat = (fat_t *)fs->data;

    while (dir->index<fat->root_ent_cnt)
    {
        diritem_t * item = read_dir_entry(fat, dir->index);
        if (item == (diritem_t *) 0)
        {
            return -1;
        }
        if (item->DIR_Name[0] == DIRITEM_NAME_END)
        {
            break;
        }
        if (item->DIR_Name[0] != DIRITEM_NAME_FREE)
        {
            file_type_t type = diritem_get_type(item);
            if ((type == FILE_NORMAL) || (type == FILE_DIR))
            {
                dirent->size = item->DIR_FileSize;
                dirent->type = type;
                diritem_get_name(item, dirent->name);
                dirent->index = dir->index++;
                return 0;
            } 
        }
        dir->index++;
    }
    
    return -1;
}

int fatfs_closedir(struct _fs_t * fs, DIR * dir)
{
    return 0;
}


fs_op_t fatfs_op = {
    .mount = fatfs_mount,
    .unmount = fatfs_unmount,
    .open = fatfs_open,
    .read = fatfs_read,
    .write = fatfs_write,
    .close = fatfs_close,
    .seek = fatfs_seek,
    .stat = fatfs_stat,
    .opendir = fatfs_opendir,
    .readdir = fatfs_readdir,
    .closedir = fatfs_closedir,
};
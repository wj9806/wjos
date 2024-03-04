#ifndef __FS_H__
#define __FS_H__

#include <sys/stat.h>
#include "fs/file.h"
#include "tools/list.h"
#include "ipc/mutex.h"

#define FS_MOUNT_SIZE           512

struct _fs_t;

typedef struct _fs_op_t
{
    int (*mount)(struct _fs_t * fs, int major, int minor);
    void (*unmount)(struct _fs_t * fs);
    int (*open)(struct _fs_t * fs, const char * path, file_t * file);
    int (*read)(char * buf, int size, file_t * file);
    int (*write)(char * buf, int size, file_t * file);
    void (*close)(file_t * file);
    int (*seek)(file_t * file, uint32_t offset, int dir);
    int (*stat)(file_t * file, struct stat * st);
} fs_op_t;

typedef enum _fs_type_t
{
    FS_DEVFS,
} fs_type_t;

typedef struct _fs_t 
{
    char mount_point[FS_MOUNT_SIZE];
    fs_type_t type;
    fs_op_t * op;
    void * data;
    int dev_id;
    node_t node;
    mutex_t * mutex;
} fs_t;

void fs_init(void);

int sys_open(const char * name, int flags, ...);

int sys_read(int file, char * ptr, int len);

int sys_write(int file, char * ptr, int len);

int sys_lseek(int file, int ptr, int dir);

int sys_close(int file);

int sys_isatty(int file);

int sys_fstat(int file, struct stat * st);

int sys_dup(int file);

#endif
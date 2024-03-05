#ifndef __FILE_H__
#define __FILE_H__

#include "comm/types.h"

#define FILE_TABLE_SIZE     512
#define FILE_NAME_SIZE      32

typedef enum _file_type_t
{
    FILE_UNKNOWN = 0,
    FILE_TTY = 1,
    FILE_DIR = 2,
    FILE_NORMAL = 3,
} file_type_t;

struct _fs_t;
typedef struct _file_t
{
    char file_name[FILE_NAME_SIZE];
    file_type_t type;
    uint32_t size;
    //被打开的次数
    int ref;
    //所属的设备
    int dev_id;
    //当前读取的位置
    int pos;
    int mode;

    struct _fs_t * fs;
} file_t;

file_t * file_alloc(void);

void file_free(file_t * file);

void file_table_init(void);

void file_inc_ref(file_t * file);

#endif
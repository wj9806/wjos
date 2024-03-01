#ifndef __FILE_H__
#define __FILE_H__

#include "comm/types.h"

#define FILE_TABLE_SIZE     2048
#define FILE_NAME_SIZE      127

typedef enum _file_type_t
{
    FILE_UNKNOWN = 0,
    FILE_TTY = 1,
} file_type_t;


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
} file_t;

file_t * file_alloc(void);

void file_free(file_t * file);

void file_table_init(void);

#endif
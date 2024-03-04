#ifndef __DISK_H__
#define __DISK_H__

#include "comm/types.h"

#define DISK_NAME_SIZE              32
#define DISK_PRIMARY_PART_CNT       (4 + 1)
#define PART_NAME_SIZE              32
#define DISK_CNT                    2
#define DISK_PER_CHANNEL            2

struct _disk_t;
typedef struct _partinfo_t
{
    char name[PART_NAME_SIZE];
    struct _disk_t * disk;
    int start_sector;
    int total_sector;
    enum {
        FS_INVALID = 0x00,
        FS_FAT16_0 = 0x6, 
        FS_FAT16_1 = 0xE, 
    } type;
} partinfo_t;

typedef struct _disk_t
{
    char name[DISK_NAME_SIZE];
    enum {DISK_MASTER, DISK_SLAVE} drive;
    uint16_t port_base;
    int sector_size;
    int sector_count;
    partinfo_t partinfo[DISK_PRIMARY_PART_CNT];
} disk_t;

void disk_init(void);

#endif
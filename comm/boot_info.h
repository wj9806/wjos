#ifndef __BOOT_INFO_H__
#define __BOOT_INFO_H__

#include "types.h"

#define BOOT_RAM_REGION_MAX  10

typedef struct _boot_info_t
{
    struct
    {
        uint32_t start;
        uint32_t size;
    } ram_region_cfg[BOOT_RAM_REGION_MAX];

    int ram_region_count;
    
} boot_info_t;

#define SYS_KERNEL_LOAD_ADDR (1024 * 1024)
#define SECTOR_SIZE 512

#endif
#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "comm/boot_info.h"
#include "comm/types.h"
#include "tools/bitmap.h"
#include "ipc/mutex.h"

#define MEM_EXT_START           (1024*1024)
#define MEM_PAGE_SIZE           4096
#define MEM_EBDA_START          0x80000

typedef struct _addr_alloc_t
{
    mutex_t mutex;
    bitmap_t bitmap;
    uint32_t start;
    uint32_t size;
    //页大小
    uint32_t page_size;
} addr_alloc_t;

typedef struct _memory_map_t
{
    void * vstart;
    void * vend;
    void * pstart;

    uint32_t perm;
} memory_map_t;


void memory_init(boot_info_t * boot_info);

#endif
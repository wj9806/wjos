#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "comm/boot_info.h"
#include "comm/types.h"
#include "tools/bitmap.h"
#include "ipc/mutex.h"

#define MEM_EXT_START           (1024*1024)
#define MEM_EXT_END             (127 * 1024 * 1024)
#define MEM_PAGE_SIZE           4096
#define MEM_EBDA_START          0x80000
#define MEMORY_TASK_BASE        0X80000000

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

uint32_t memory_create_uvm();

void memory_init(boot_info_t * boot_info);

int memory_alloc_page_for(uint32_t vaddr, uint32_t size, int perm);
#endif
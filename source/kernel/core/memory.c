#include "core/memory.h"
#include "tools/log.h"
#include "tools/klib.h"

/**
 * 系统内存分两大块 1M以内给操作系统，1M以上给进程使用
 * ----------------------------------------------------------------------------
 * |  |      |   |        |  |          |  |          |   |                   |  
 * |  | boot |   | loader |  |  kernel  |  |  位图缓存 |   | EBDA/显存保留区域  |
 * |  |      |   |        |  |          |  |          |   |                   |
 * ----------------------------------------------------------------------------
 *    0x7c00      0x8000       0x10000          ||              0x8000~0xFFFF
 *                                              || 映射
 *                                              \/
 * --------------------------------------------------------------------------
 * |        |       |       |       |       |       |       |       |       |
 * |        |       |       |       |       |       |       |       |       |
 * |        |       |       |       |       |       |       |       |       |
 * --------------------------------------------------------------------------                                               
 *  0x100000 
 * 
*/

//位图缓存
static addr_alloc_t paddr_alloc;

static void addr_alloc_init(addr_alloc_t * addr_alloc, uint8_t * bits, 
    uint32_t start, uint32_t size, uint32_t page_size)
{
    mutex_init(&addr_alloc->mutex);
    addr_alloc->start = start;
    addr_alloc->size = size;
    addr_alloc->page_size = page_size;
    bitmap_init(&addr_alloc->bitmap, bits, addr_alloc->size/ page_size, 0);
}

//页分配
static uint32_t addr_alloc_page(addr_alloc_t* alloc, int page_count)
{
    uint32_t addr = 0;
    mutex_lock(&alloc->mutex);

    int page_index = bitmap_alloc_nbits(&alloc->bitmap, 0, page_count);
    if (page_index >= 0)
    {
        addr = alloc->start + page_index * alloc->page_size;
    }
    
    mutex_unlock(&alloc->mutex);
    return addr;
}

//释放页
static void addr_free_page(addr_alloc_t* alloc, uint32_t addr, int page_count)
{
    mutex_lock(&alloc->mutex);
    uint32_t page_index = (addr - alloc->start) / alloc->page_size;
    bitmap_set_bit(&alloc->bitmap, page_index, page_count, 0);
    mutex_unlock(&alloc->mutex);
}

void show_mem_info(boot_info_t * boot_info)
{
    log_printf("memory region:");
    for (int i = 0; i < boot_info->ram_region_count; i++)
    {
        log_printf("[%d]: 0x%x - 0x%x", i, boot_info->ram_region_cfg[i].start, 
            boot_info->ram_region_cfg[i].size);
    }
}

static uint32_t total_memory_size(boot_info_t * boot_info)
{
    uint32_t mem_size = 0;
    for (int i = 0; i < boot_info->ram_region_count; i++)
    {
        mem_size += boot_info->ram_region_cfg[i].size;
    }
    return mem_size;
}

/**
 * 
 * 1、初始化物理内存分配器：将所有物理内存管理起来. 在1MB内存中分配物理位图
 * 2、重新创建内核页表：原loader中创建的页表已经不再合适
*/
void memory_init(boot_info_t * boot_info)
{
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   在1MB内存中分配物理位图  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    extern uint8_t * mem_free_start;

    log_printf("memory init");
    show_mem_info(boot_info);

    // 在内核数据后面放物理页位图
    uint8_t * mem_free = (uint8_t *) &mem_free_start;

    // 计算1MB以上空间的空闲内存容量，并对齐的页边界
    uint32_t mem_up1MB_free = total_memory_size(boot_info) - MEM_EXT_START;
    mem_up1MB_free = down2(mem_up1MB_free, MEM_PAGE_SIZE);
    
    log_printf("free memory: 0x%x, size: 0x%x", MEM_EXT_START, mem_up1MB_free);

    // 4GB大小需要总共4*1024*1024*1024/4096/8=128KB的位图, 使用低1MB的RAM空间中足够
    // 该部分的内存仅跟在mem_free_start开始放置
    addr_alloc_init(&paddr_alloc, mem_free, MEM_EXT_START, mem_up1MB_free, MEM_PAGE_SIZE);
    mem_free += bitmap_byte_count(paddr_alloc.size / MEM_PAGE_SIZE);

    // 到这里，mem_free应该比EBDA地址要小
    ASSERT(mem_free < (uint8_t *)MEM_EBDA_START);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
}
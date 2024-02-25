#include "core/memory.h"
#include "cpu/mmu.h"
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

static pde_t kernel_page_dir[PDE_CNT] __attribute__((aligned(MEM_PAGE_SIZE)));

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

pte_t * find_pte(pde_t* page_dir, uint32_t vaddr, int alloc)
{
    pte_t * page_table;
    pde_t * pde = page_dir + pde_index(vaddr);
    if (pde->present)
    {
        page_table = (pte_t *)pde_paddr(pde);
    }
    else{
        if (alloc == 0)
        {
            return (pte_t *)0;
        }
        else
        {
            uint32_t pg_addr = addr_alloc_page(&paddr_alloc, 1);
            if (pg_addr == 0)
            {
                return (pte_t *)0;
            }
            pde->v = pg_addr | PDE_P | PDE_W | PDE_U; 
            
            page_table = (pte_t*) pg_addr;
            kernel_memset(page_table, 0, MEM_PAGE_SIZE);
        }
    }
    
    return page_table + pte_index(vaddr);
}

/**
 * page_dir 页表指针
 * vaddr 虚拟地址的开始
 * paddr 物理地址的开始
 * count 物理页
*/
int memory_create_map(pde_t * page_dir, uint32_t vaddr, uint32_t paddr, int count, uint32_t perm)
{
    for (int i = 0; i < count; i++)
    {
        //log_printf("create map: v-0x%x, p-0x%x, perm:0x%x", vaddr, paddr, perm);
        pte_t * pte = find_pte(page_dir, vaddr, 1);
        if (pte == (pte_t *) 0)
        {
            //log_printf("create pte table failed!");
            return -1;
        }

        //log_printf("pte addr:0x%x", (uint32_t)pte);
        ASSERT(pte->present == 0);

        pte->v = paddr | perm | PTE_P;
        vaddr += MEM_PAGE_SIZE;
        paddr += MEM_PAGE_SIZE;
    }
    return 0;
}

void create_kernel_table(void)
{
    //参考kernel.lds文件
    extern uint8_t s_text[], e_text[], s_data[];
    extern uint8_t kernel_base[];
    static memory_map_t kernel_map[] = {
        {kernel_base,           s_text,                    kernel_base,            PTE_W},
        {s_text,                e_text,                    s_text,                 0},
        {s_data,                (void*)MEM_EBDA_START,     s_data,                 PTE_W},
        {(void*)MEM_EXT_START,  (void*)MEM_EXT_END,        (void*)MEM_EXT_START,   PTE_W}
    };

    for (int i = 0; i < sizeof(kernel_map) / sizeof(memory_map_t); i++)
    {
        memory_map_t * map = kernel_map + i;
        uint32_t vstart = down2((uint32_t)map->vstart, MEM_PAGE_SIZE);
        uint32_t vend = up2((uint32_t)map->vend, MEM_PAGE_SIZE);
        uint32_t paddr = down2((uint32_t)map->pstart, MEM_PAGE_SIZE);
        int page_count = (vend - vstart) / MEM_PAGE_SIZE;

        //虚拟地址和物理地址建立关系
        memory_create_map(kernel_page_dir, vstart, (uint32_t)paddr, page_count, map->perm);
    }

}

//创建用户虚拟内存
uint32_t memory_create_uvm()
{
    pde_t * page_dir = (pde_t *) addr_alloc_page(&paddr_alloc, 1);
    if (page_dir == 0)
    {
        return 0;
    }
    kernel_memset((void *) page_dir, 0, MEM_PAGE_SIZE);
    uint32_t user_pde_start = pde_index(MEMORY_TASK_BASE);
    for (int i = 0; i < user_pde_start; i++)
    {
        page_dir[i].v = kernel_page_dir[i].v;
    }
    
    return (uint32_t) page_dir;
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

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  创建内核页表  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    create_kernel_table();
    //设置页表 打开分页机制
    mmu_set_page_dir((uint32_t)kernel_page_dir);

}

int memory_alloc_for_page_dir(uint32_t page_dir, uint32_t vaddr, uint32_t size, int perm)
{
    uint32_t curr_vaddr = vaddr;
    int page_count =  up2(size, MEM_PAGE_SIZE) / MEM_PAGE_SIZE;
    for (int i = 0; i < page_count; i++)
    {
        uint32_t paddr = addr_alloc_page(&paddr_alloc, 1);
        if (paddr == 0)
        {
            log_printf("mem alloc failed. no free memory");
            return 0;
        }
        int err = memory_create_map((pde_t*)page_dir, curr_vaddr, paddr, 1, perm);
        if (err < 0)
        {
            log_printf("create memory failed. err = %d", err);
            addr_free_page(&paddr_alloc, vaddr, i + 1);
            return 0;           
        }
        curr_vaddr += MEM_PAGE_SIZE;
    }
    return 0;
}

int memory_alloc_page_for(uint32_t vaddr, uint32_t size, int perm)
{
    return memory_alloc_for_page_dir(task_current()->tss.cr3, vaddr, size, perm);
}

uint32_t memory_alloc_page(void)
{
    uint32_t addr = addr_alloc_page(&paddr_alloc, 1);
    return addr;
}

static pde_t * curr_page_dir(void)
{
    return (pde_t *)(task_current()->tss.cr3);
}

void memory_free_page(uint32_t addr)
{
    if (addr < MEMORY_TASK_BASE)
    {
        addr_free_page(&paddr_alloc, addr, 1);
    }
    else
    {
        pte_t * pte = find_pte(curr_page_dir(), addr, 0);
        ASSERT(pte != (pte_t *)0 && pte->present);
        addr_free_page(&paddr_alloc, pte_paddr(pte), 1);
        pte->v = 0;
    }
    
}

uint32_t memory_copy_uvm(uint32_t page_dir)
{
    uint32_t to_page_dir = memory_create_uvm();
    if (to_page_dir == 0)
    {
        goto copy_uvm_failed;
    }
    uint32_t user_pde_start = pde_index(MEMORY_TASK_BASE);
    //获取需要被复制的表项
    pde_t * pde = (pde_t *)page_dir + user_pde_start;
    for (int i = user_pde_start; i < PDE_CNT; i++, pde++)
    {
        if (!pde->present)
        {
            continue;
        }
        
        pte_t * pte = (pte_t *)pde_paddr(pde);
        for (int j = 0; j < PTE_CNT; j++, pte++)
        {
            if (!pte->present)
            {
                continue;
            }
            uint32_t page = addr_alloc_page(&paddr_alloc, 1);
            if (page == 0)
            {
                goto copy_uvm_failed;
            }
            uint32_t vaddr = (i << 22) | (j <<12); 
            int err = memory_create_map((pde_t *)to_page_dir, 
                    vaddr, page, 1, get_pte_perm(pte));
            if (err < 0)
            {
                goto copy_uvm_failed;
            }
            kernel_memcpy((void *)page, (void *)vaddr, MEM_PAGE_SIZE);
        }   
    }
    return to_page_dir;
copy_uvm_failed:
    if (to_page_dir)
    {
        memory_destroy_uvm(to_page_dir);
    }
    return 0;
}

void memory_destroy_uvm(uint32_t page_dir)
{
    uint32_t user_pde_start = pde_index(MEMORY_TASK_BASE);
    pde_t * pde = (pde_t *)page_dir + user_pde_start;
    for (int i = user_pde_start; i < PDE_CNT; i++, pde++)
    {
        if (!pde->present)
        {
            continue;
        }
        pte_t * pte = (pte_t *)pde_paddr(pde);
        for (int j = 0; j < PTE_CNT; j++, pte++)
        {
            if (!pte->present)
            {
                continue;
            }
            addr_free_page(&paddr_alloc, pte_paddr(pte), 1);
        }
        addr_free_page(&paddr_alloc, (uint32_t)pde_paddr(pde), 1);
    }

    addr_free_page(&paddr_alloc, page_dir, 1);
}
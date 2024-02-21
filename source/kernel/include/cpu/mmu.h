//虚拟内存管理单元
#ifndef __MMU_H__
#define __MMU_H__

#include "comm/types.h"
#include "comm/cpu_instr.h"

#define PDE_CNT         1024
#define PTE_P           (1 << 0)
#define PDE_P           (1 << 0)
#define PDE_W           (1 << 1)
#define PTE_W           (1 << 1)
#define PDE_U           (1 << 2)

//页目录
typedef union _pde_t
{
    uint32_t v;
    struct
    {
        uint32_t present : 1;
        uint32_t write_disable : 1;
        uint32_t user_mode_acc : 1;
        uint32_t write_through : 1;
        uint32_t cache_disable : 1;
        uint32_t accessed : 1;
        uint32_t : 1;
        uint32_t ps : 1;
        uint32_t : 4;
        uint32_t phy_pt_addr: 20;
    };
    
} pde_t;

//页表
typedef union _pte_t
{
    uint32_t v;
    struct
    {
        uint32_t present : 1;
        uint32_t write_disable : 1;
        uint32_t user_mode_acc : 1;
        uint32_t write_through : 1;
        uint32_t cache_disable : 1;
        uint32_t accessed : 1;
        uint32_t dirty : 1;
        uint32_t pat : 1;
        uint32_t global : 1;
        uint32_t : 3;
        uint32_t phy_pt_addr: 20;
    };
    
} pte_t;

/**
 * @brief 返回vaddr在页目录中的索引
 */
static inline uint32_t pde_index (uint32_t vaddr) {
    int index = (vaddr >> 22); // 只取高10位
    return index;
}

/**
 * @brief 获取pde中地址
 */
static inline uint32_t pde_paddr (pde_t * pde) {
    return pde->phy_pt_addr << 12;
}

/**
 * @brief 返回vaddr在页表中的索引
 */
static inline int pte_index (uint32_t vaddr) {
    return (vaddr >> 12) & 0x3FF;   // 取中间10位
}

/**
 * @brief 获取pte中的物理地址
 */
static inline uint32_t pte_paddr (pte_t * pte) {
    return pte->phy_pt_addr << 12;
}

/**
 * @brief 获取pte中的权限位
 */
static inline uint32_t get_pte_perm (pte_t * pte) {
    return (pte->v & 0x3FF);
}

/**
 * @brief 重新加载整个页表
 */
static inline void mmu_set_page_dir (uint32_t paddr) {
    // 将虚拟地址转换为物理地址
    write_cr3(paddr);
}

#endif
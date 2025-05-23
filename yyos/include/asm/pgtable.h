#ifndef ASM_PGTABLE_H
#define ASM_PGTABLE_H

#include "asm/barrier.h"
#include <asm/pgtable_hwdef.h>
#include "asm/pgtable_types.h"

/* 查找PGD索引 */
/*右移找到地址的相应的段，然后与上高位掩码找到在PGD中的位域*/
#define pgd_index(addr)  (((addr) >> PGDIR_SHIFT) &(PTR_PER_PGD - 1))

/* 通过地址addr查找PGD的表项 */
#define pgd_offset_raw(pgd, addr) ((pgd) + pgd_index(addr))


/*通过虚拟地址找到下一层级页表的开始地址也是这一层级页表的结束地址*/
#define pgd_addr_end(addr,end)     \
({                                                  \        
    unsigned long __boundary = ((addr) + PGDIR_SIZE) & PGDIR_MASK; \
    (__boundary-1 < (end) -1) ? __boundary : (end);    \
})



#define pud_addr_end(addr, end)						\
({	unsigned long __boundary = ((addr) + PUD_SIZE) & PUD_MASK;	\
	(__boundary - 1 < (end) - 1) ? __boundary : (end);		\
})

#define pmd_addr_end(addr, end)						\
({	unsigned long __boundary = ((addr) + PMD_SIZE) & PMD_MASK;	\
	(__boundary - 1 < (end) - 1) ? __boundary : (end);		\
})

/*检查表项内容是否为空*/
#define pgd_none(pgd) (!pgd_val(pgd))
#define pud_none(pud) (!pud_val(pud))
#define pmd_none(pmd) (!pmd_val(pmd))
#define ptd_none(ptd) (!ptd_val(ptd))

/*判断页表是否指向快内存*/
#define pmd_sect(pmd)	((pmd_val(pmd) & PMD_TYPE_MASK) == \
				 PMD_TYPE_SECT)
#define pud_sect(pud)	((pud_val(pud) & PUD_TYPE_MASK) == \
				 PUD_TYPE_SECT)


/*获取这个页表的偏移地址*/
static inline unsigned long pgd_page_paddr(pgd_t pgd)
{
    return pgd_val(pgd) & PTE_ADDR_MASK;
}       


/*查找PUD索引*/
/*根据虚拟地址，找到此时如果PUD指向快内存的话的物理地址*/
/*物理地址等于  偏移地址加上索引乘上它的单位字节大小*/
#define pud_index(addr) ((addr) >> PUD_SHIFT & (PTR_PER_PUD - 1))
#define pud_offset_phys(pgd, addr) ((pud_t *)((pgd_page_paddr(*(pgd)) + pud_index(addr) * sizeof(pud_t))))

/*获取这个页表的偏移地址*/
static inline unsigned long pud_page_paddr(pud_t pud)
{
	return pud_val(pud) & PTE_ADDR_MASK;
}


#define pmd_index(addr) ((addr) >> PMD_SHIFT & (PTR_PER_PMD - 1))
#define pmd_offset_phys(pud, addr) ((pmd_t *)(pud_page_paddr(*(pud)) + pmd_index(addr) * sizeof(pmd_t)))

#define pfn_pmd(pfn, prot) (__pmd(((pfn) << PMD_SHIFT) | (prot)))


/*获取这个页表的偏移地址*/
static inline unsigned long pmd_page_paddr(pmd_t pmd)
{
	return pmd_val(pmd) & PTE_ADDR_MASK;
}

#define pte_index(addr) (((addr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))
#define pte_offset_phys(dir, addr) ((pte_t *)(pmd_page_paddr(*(dir)) + pte_index(addr) * sizeof(pte_t)))

/*将物理页帧号（PFN）和页表属性（prot）组合成页表条目（PTE）的工具*/
#define pfn_pte(pfn, prot)  (__pte(((pfn) << PAGE_SHIFT) | (prot)))


static inline void set_pgd(pgd_t *pgdp, pgd_t pgd)
{
	*pgdp = pgd;

	dsb(ishst);
}

static inline void set_pud(pud_t *pudp, pud_t pud)
{
	*pudp = pud;

	dsb(ishst);
}

static inline void set_pmd(pmd_t *pmdp, pmd_t pmd)
{
	*pmdp = pmd;

	dsb(ishst);
}

static inline void set_pte(pte_t *ptep, pte_t pte)
{
	*ptep = pte;
	dsb(ishst);
}

static inline unsigned long mk_sect_prot(unsigned long prot)
{
	return prot & ~PTE_TABLE_BIT;
}




#endif
#include <asm/pgtable.h>
#include <asm/pagetable_prot.h>
#include <asm/pgtable_hwdef.h>
#include <sysregs.h>
#include <asm/barrier.h>
#include <string.h>
#include <asm/base.h>
#include "page_alloc.h"
#include "rw_regs.h"



#define NO_BLOCK_MAPPINGS BIT(0)
#define NO_CONT_MAPPINGS BIT(1)

extern char idmap_pg_dir[];//4KB的初始PGD页表

extern char text_boot_start[], text_boot_end[];
extern char text_start[], text_end[];



static void alloc_init_pte(pmd_t *pmdp, unsigned long addr,
                        unsigned long end, unsigned long phys,
                        unsigned long prot,
                        unsigned long (*alloc_pgtable)(void),
                        unsigned long flags)
{
    pmd_t pmd = *pmdp;
    pte_t *ptep;

    if(pmd_none(pmd))
    {
        unsigned long pte_phys;

        pte_phys = alloc_pgtable();
        set_pmd(pmdp, __pmd(pte_phys | PMD_TYPE_TABLE));
        pmd = *pmdp;
    }

    /*填充pte*/
    /*多个页表项填充*/
    ptep = pte_offset_phys(pmdp, addr);
    do{
        set_pte(ptep, pfn_pte(phys >> PAGE_SHIFT, prot));
        phys += PAGE_SIZE;
    }while(ptep++, addr += PAGE_SIZE, addr != end);
}



/*映射快内存*/
void pmd_set_section(pmd_t *pmdp, unsigned long phys, unsigned long prot)
{
    //组合成为这个段页表项的属性
    unsigned long sect_prot = PMD_TYPE_SECT | mk_sect_prot(prot);

    //将物理地址右移12位变成pmd的pfn然后或上页表项属性成为新的页表项    
    pmd_t mew_pmd = pfn_pmd(phys >> PMD_SHIFT, sect_prot);

    set_pmd(pmdp,mew_pmd);//填充新的组合后的页表项

}



static void alloc_init_pmd (pud_t *pudp, unsigned long addr,
                        unsigned long end, unsigned long phys,
                        unsigned long prot,
                        unsigned long (*alloc_pgtable)(void),
                        unsigned long flags)
{
    pud_t pud = *pudp;//通过索引地址指向页表项内容
    pmd_t *pmdp;//定义一个页表项地址
    unsigned long next;//

    /*通过此时传递进来的pgd页表的索引的地址,依次检查并且填充*/
    if(pud_none(pud))
    {
        unsigned long pmd_phys;
        pmd_phys = alloc_pgtable();
        set_pud(pudp,__pud(pmd_phys | PUD_TYPE_TABLE));//此时页表项内容是页表类型
        pud = *pudp;//重新指向此时填充好的页表项
    }

    pmdp = pmd_offset_phys(pudp, addr);//需要前一级的页表项的pgd的偏移地址

    do{
        next =  pmd_addr_end(addr, end);//找到下一个表项的起始地址
        if(((addr | next | phys) & ~SECTION_MASK) == 0 && 
            (flags & NO_BLOCK_MAPPINGS) == 0)//如果要映射的虚拟地址和物理地址都为2MB对齐的话
            //并且标志为上述标志
            {
                pmd_set_section(pmdp, phys, prot);//映射大内存
            }
        else
        {
            alloc_init_pte(pmdp, addr, next, phys,
                prot,  alloc_pgtable, flags);
        }
        phys += next - addr;//实际物理地址也要加上跨越
    } while(pmdp++, addr=next, addr!= end);
}



/*填充pgd的表项，并且开始填充pud表项*/
static void alloc_init_pud(pgd_t *pgdp, unsigned long addr,
                        unsigned long end, unsigned long phys,
                        unsigned long prot,
                        unsigned long (*alloc_pgtable)(void),
                        unsigned long flags)
{
    pgd_t pgd = *pgdp;//通过索引地址指向页表项内容
    pud_t *pudp;
    unsigned long next;


    /*通过此时传递进来的pgd页表的索引的地址,依次检查并且填充*/
    if(pgd_none(pgd))
    {
        unsigned long pud_phys;
        pud_phys = alloc_pgtable();
        set_pgd(pgdp,__pgd(pud_phys | PUD_TYPE_TABLE));//此时页表项内容是页表类型
        pgd = *pgdp;//重新指向此时填充好的页表项
    }

    pudp = pud_offset_phys(pgdp,addr);//需要前一级的页表项的pgd的偏移地址

    do{

        next = pud_addr_end(addr, end);//找到下一个表项的起始地址
        alloc_init_pmd(pudp, addr, next, phys,
            prot, alloc_pgtable, flags);//根据此时的pud表项找到pmd的基地址，并且填充pud的表项
        
        phys += next - addr;//实际物理地址也要加上跨越
    } while(pudp++, addr=next, addr!= end);


}



/*开始填充各级页表*/
static void __create_pgd_mapping(pgd_t *pgdir, unsigned long phys,
                 unsigned long virt, unsigned long size,
                 unsigned long prot,
                 unsigned long (*alloc_pgtable)(void),
                 unsigned long flags)
{
    pgd_t *pgdp = pgd_offset_raw(pgdir,virt);//找到页表项索引地址

    unsigned long addr, end, next;

    phys &= PAGE_MASK;
	addr = virt & PAGE_MASK;
	end = PAGE_ALIGN(virt + size); 

    do{
        /*这里用next作为下一级页表项的end是因为刚好next的页表项是一整个下一级别的页表size*/
        next = pgd_addr_end(addr,end);//找到下一个表项的起始地址
        alloc_init_pud(pgdp,addr,next,phys,prot,alloc_pgtable,flags);
        phys += next - addr;//实际物理地址也要加上跨越
    } while(pgdp++, addr=next, addr!= end);



}


/*申请一个4K页面的地址*/
static unsigned long early_pgtable_alloc(void)
{
    unsigned long phys;
    phys = get_free_page();

	memset((void *)phys, 0, PAGE_SIZE);

	return phys;
}


/*开始在代码段做恒等映射*/
static void create_identical_mapping(void)
{   
    unsigned long start;
    unsigned long end;

    /*map text*/
	start = (unsigned long)text_boot_start;
	end = (unsigned long)text_end;
    __create_pgd_mapping((pgd_t *)idmap_pg_dir, start,start,
                        end-start, PAGE_KERNEL_ROX,
                        early_pgtable_alloc, 0);
                     
    /*map memory*/     
    start = PAGE_ALIGN((unsigned long)text_end);
	end = TOTAL_MEMORY;
	__create_pgd_mapping((pgd_t *)idmap_pg_dir, start, start,
			end - start, PAGE_KERNEL,
			early_pgtable_alloc,
			0);                  

}


/*cpu中的外设填充*/
static void create_mmio_mapping(void)
{
    __create_pgd_mapping((pgd_t *)idmap_pg_dir, PBASE, PBASE,
    DEVICE_SIZE, PROT_DEVICE_nGnRnE,
    early_pgtable_alloc,
    0);
}

/*mmu相关寄存器初始化*/
static void cpu_mmu_init(void)
{
    //定义几个64位的寄存器
    unsigned long mair = 0;
    unsigned long tcr = 0;
    unsigned long tmp;
	unsigned long parang;


    asm("tlbi vmalle1");
	dsb(nsh);//内存屏障指令


    //控制 EL0/EL1 对浮点（FP）、NEON（SIMD）和 SME 功能的访问权限。
    write_sysreg(3UL << 20, cpacr_el1);
    //控制调试相关的行为（如断点、单步执行）。
    write_sysreg(1 << 12, mdscr_el1);

    mair = MAIR(0x00UL, MT_DEVICE_nGnRnE) |  // Index 0: 严格设备内存（无聚合、无重排序、无早期写确认）
    MAIR(0x04UL, MT_DEVICE_nGnRE) |   // Index 1: 设备内存（无聚合、无重排序）
    MAIR(0x0cUL, MT_DEVICE_GRE) |     // Index 3: 设备内存（可聚合）
    MAIR(0x44UL, MT_NORMAL_NC) |      // Index 8: 非缓存普通内存
    MAIR(0xffUL, MT_NORMAL) |         // Index 15: 回写（Write-Back）普通内存
    MAIR(0xbbUL, MT_NORMAL_WT);       // Index 11: 写通（Write-Through）普通内存
    write_sysreg(mair, mair_el1);


    //配置TTRB1_EL1（映射用户空间）和TTRB0_EL1（映射内核）管辖的大小
    //TCR_TG_FLAGS表示设置页面大小为4K
    tcr = TCR_TxSZ(VA_BITS) | TCR_TG_FLAGS;

    tmp = read_sysreg(ID_AA64MMFR0_EL1);

    parang = tmp & 0xf;
    //查看是否支持48位总线
	if (parang > ID_AA64MMFR0_PARANGE_48)
    {
        parang = ID_AA64MMFR0_PARANGE_48;
    }
		
    //设置IPS字段，配置物理地址的大小，即48bit，256TB大小空间
	tcr |= parang << TCR_IPS_SHIFT;//

	write_sysreg(tcr, tcr_el1);


}

static int enable_mmu(void)
{
    unsigned long tmp;
    int tgran4;
    
    /*确认当前 CPU 是否支持 4KB 大小的页表粒度（Translation Granule），
    这是 MMU 配置的基*/
    tmp = read_sysreg(ID_AA64MMFR0_EL1);
    tgran4 = (tmp >> ID_AA64MMFR0_TGRAN4_SHIFT) & 0xf;
    if(tgran4 != ID_AA64MMFR0_TGRAN4_SUPPORTED)
    {
        return -1;
    }
    write_sysreg(idmap_pg_dir,ttbr0_el1);//将第一级页表写入寄存器

    isb();

    write_sysreg(SCTLR_ELx_M, sctlr_el1);//用来设置SCTLR_EL1寄存器，打开MMU
    isb();

    /*cache指令*/
    asm("ic iallu");
	dsb(nsh);
	isb();

    return 0;
}


/*软件开始做映射填充*/
void paging_init(void)
{

    printk("!!\n");
    memset(idmap_pg_dir,0,PAGE_SIZE);
    create_identical_mapping();
	create_mmio_mapping();
	cpu_mmu_init();
	enable_mmu();

	printk("enable mmu done\n");

}









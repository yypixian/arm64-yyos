#include <asm/pgtable.h>
#include <asm/pagetable_prot.h>
#include <asm/pgtable_hwdef.h>
#include "mm.h"

extern char idmap_pg_dir[];


struct addr_marker {
	unsigned long start_address;
	char *name;
};

struct prot_bits {
    unsigned long mask;
	unsigned long val;//属性值
	const char *set;//脏位
	const char *clear;//净位
};


/*表示dump出来的页表*/
struct pg_level {
    const struct prot_bits *bits;//页表属性 
    const char *name;//页表属性
    size_t num;//页表个数
    unsigned long mask;//
};


/*页表的属性*/
static const struct prot_bits pte_bits[] = {
	{
		.mask	= PTE_VALID,
		.val	= PTE_VALID,
		.set	= " ",
		.clear	= "F",
	}, {
		.mask	= PTE_USER,
		.val	= PTE_USER,
		.set	= "USR",
		.clear	= "   ",
	}, {
		.mask	= PTE_RDONLY,
		.val	= PTE_RDONLY,
		.set	= "ro",
		.clear	= "RW",
	}, {
		.mask	= PTE_PXN,
		.val	= PTE_PXN,
		.set	= "NX",
		.clear	= "x ",
	}, {
		.mask	= PTE_SHARED,
		.val	= PTE_SHARED,
		.set	= "SHD",
		.clear	= "   ",
	}, {
		.mask	= PTE_AF,
		.val	= PTE_AF,
		.set	= "AF",
		.clear	= "  ",
	}, {
		.mask	= PTE_NG,
		.val	= PTE_NG,
		.set	= "NG",
		.clear	= "  ",
	}, {
		.mask	= PTE_CONT,
		.val	= PTE_CONT,
		.set	= "CON",
		.clear	= "   ",
	}, {
		.mask	= PTE_TABLE_BIT,
		.val	= PTE_TABLE_BIT,
		.set	= "   ",
		.clear	= "BLK",
	}, {
		.mask	= PTE_UXN,
		.val	= PTE_UXN,
		.set	= "UXN",
	}, {
		.mask	= PTE_ATTRINDX_MASK,
		.val	= PTE_ATTRINDX(MT_DEVICE_nGnRnE),
		.set	= "DEVICE/nGnRnE",
	}, {
		.mask	= PTE_ATTRINDX_MASK,
		.val	= PTE_ATTRINDX(MT_DEVICE_nGnRE),
		.set	= "DEVICE/nGnRE",
	}, {
		.mask	= PTE_ATTRINDX_MASK,
		.val	= PTE_ATTRINDX(MT_DEVICE_GRE),
		.set	= "DEVICE/GRE",
	}, {
		.mask	= PTE_ATTRINDX_MASK,
		.val	= PTE_ATTRINDX(MT_NORMAL_NC),
		.set	= "MEM/NORMAL-NC",
	}, {
		.mask	= PTE_ATTRINDX_MASK,
		.val	= PTE_ATTRINDX(MT_NORMAL),
		.set	= "MEM/NORMAL",
	}
};

/*四级页表*/
static struct pg_level pg_level[] = {
	{
	}, { /* pgd */
		.name	= "PGD",
		.bits	= pte_bits,
		.num	= ARRAY_SIZE(pte_bits),
	}, { /* pud */
		.name	= "PUD",
		.bits	= pte_bits,
		.num	= ARRAY_SIZE(pte_bits),
	}, { /* pmd */
		.name	= "PMD",
		.bits	= pte_bits,
		.num	= ARRAY_SIZE(pte_bits),
	}, { /* pte */
		.name	= "PTE",
		.bits	= pte_bits,
		.num	= ARRAY_SIZE(pte_bits),
	},
};

static const struct addr_marker address_markers[] = {
	{ 0, "Identical mapping" },
};


/*打印此时的脏位信息*/
static void dump_prot(unsigned long prot, const struct prot_bits *bits,
    size_t num)
{
    unsigned i;

    for (i = 0; i < num; i++, bits++) {
        const char *s;

        if ((prot & bits->mask) == bits->val)
            s = bits->set;
        else
            s = bits->clear;

        if (s)
            printk(" %s", s);
    }
}

/*打印此时的页表项*/
static int print_pgtable(unsigned long start, unsigned long end,
    int level, unsigned long val)
{
    static const char units[] = "KMGT";
    unsigned long prot = val & pg_level[level].mask;  // 提取页表项的有效位（如物理地址+属性）
    unsigned long delta;                             // 计算地址范围大小（转换为KB/MB/GB等）
    const char *unit = units;                        // 单位（K/M/G/T）
    int i;

    for (i = 0; i < ARRAY_SIZE(address_markers); i++)
        if (start == address_markers[i].start_address)
            printk("---[ %s ]---\n", address_markers[i].name);

    if (val == 0)
        return 0;

    printk("0x%016lx-0x%016lx   ", start, end);

    delta = (end - start) >> 10;
    while (!(delta & 1023) && units[1]) {
        delta >>= 10;
        unit++;
    }

    printk("%9lu%c %s", delta, *unit,
            pg_level[level].name);

    if (pg_level[level].bits)
        dump_prot(prot, pg_level[level].bits,
                pg_level[level].num);
    printk("\n");

    return 0;
}


static void walk_pte(pmd_t *pmd, unsigned long start, unsigned long end)
{
    unsigned long addr = start;

    pte_t *ptep;//页表项地址
    pte_t pte_entry;

    ptep = pte_offset_phys(pmd, addr);

    do{
        pte_entry = *ptep;

        print_pgtable(addr, addr + PAGE_SIZE, 4, pte_val(pte_entry));
        //4表示的是PTE的页表,
    }while(ptep++,addr += PAGE_SIZE,addr!=end);
}


static void walk_pmd(pud_t *pud, unsigned long start, unsigned long end)
{
    unsigned long next, addr = start;

    pmd_t *pmdp;//页表项地址
    pmd_t pmd_entry;

    pmdp = pmd_offset_phys(pud, addr);

    do{
        pmd_entry = *pmdp;
        next = pmd_addr_end(addr,end);//找到下一个表项的起始地址
        if(pmd_none(pmd_entry) || pmd_sect(pmd_entry))
        {
            print_pgtable(addr, next, 3, pmd_val(pmd_entry));
            //3表示的是PMD的页表,
        }
        else
        {
            walk_pte(pmdp,addr,next);
        }
    }while(pmdp++,addr = next,addr!=end);
}

static void walk_pud(pgd_t *pgd, unsigned long start, unsigned long end)
{
    unsigned long next, addr = start;

    pud_t *pudp;//页表项地址
    pud_t pud_entry;

    pudp = pud_offset_phys(pgd, start);

    do{
        pud_entry = *pudp;
        next = pud_addr_end(addr,end);//找到下一个表项的起始地址
        if(pud_none(pud_entry) || pud_sect(pud_entry))
        {
            print_pgtable(addr, next, 2, pud_val(pud_entry));
            //2表示的是PUD的页表,
        }
        else
        {
            walk_pmd(pudp,addr,next);
        }
    }while(pudp++,addr = next,addr!=end);
}

static void walk_pgd(pgd_t *pdg, unsigned long start, unsigned long size)
{
    unsigned long end = start + size;
    unsigned long next, addr = start;

    pgd_t *pgdp;//页表项地址
    pgd_t pgd_entry;

    pgdp = pgd_offset_raw(pdg, addr);

    do{
        pgd_entry = *pgdp;
        next = pgd_addr_end(addr,end);//找到下一个表项的起始地址
        if(pgd_none(pgd_entry))
        {
            print_pgtable(addr, next, 1, pgd_val(pgd_entry));
            //1表示的是PGD的页表,
        }
        else
        {
            walk_pud(pgdp,addr,next);
        }
    }while(pgdp++,addr = next,addr!=end);

}



/*初始化四级页表*/
static void pg_level_init(void)
{
    unsigned long i, j;
    for(i=0;i<ARRAY_SIZE(pg_level);i++)
    {
        if(pg_level[i].bits)//即如果为有效页表项的话
        {
            for(j = 0; j < pg_level[i].num; j++)
            {
                /*一个总掩码，覆盖该级页表项的所有有效位*/
                pg_level[i].mask |= pg_level[i].bits[j].mask;//获取屏蔽位
            }
        }
    } 
}

void dump_pgtable(void)
{
    pg_level_init();
    walk_pgd((pgd_t *)idmap_pg_dir,0,TOTAL_MEMORY);
}

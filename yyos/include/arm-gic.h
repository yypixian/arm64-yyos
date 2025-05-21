/*
这个头文件定义了ARM通用中断控制器(GIC)的寄存器，
主要包括三部分：CPU接口寄存器、分发器(Distributor)寄存器和
虚拟化控制接口寄存器。
*/

/*GIC_CPU_CTRL (0x00): CPU接口控制寄存器，启用/禁用中断处理

GIC_CPU_PRIMASK (0x04): 优先级掩码寄存器，设置最低优先级阈值

GIC_CPU_BINPOINT (0x08): 二进制点寄存器，用于优先级分组

GIC_CPU_INTACK (0x0c): 中断确认寄存器，读取当前最高优先级中断

GIC_CPU_EOI (0x10): 中断结束寄存器，写入以标记中断处理完成

GIC_CPU_RUNNINGPRI (0x14): 当前运行优先级寄存器

GIC_CPU_HIGHPRI (0x18): 最高优先级挂起中断寄存器

GIC_CPU_ALIAS_BINPOINT (0x1c): 别名二进制点寄存器

GIC_CPU_ACTIVEPRIO (0xd0): 活动优先级寄存器

GIC_CPU_IDENT (0xfc): 识别寄存器*/

#define GIC_CPU_CTRL			0x00//
#define GIC_CPU_PRIMASK			0x04
#define GIC_CPU_BINPOINT		0x08
#define GIC_CPU_INTACK			0x0c
#define GIC_CPU_EOI			0x10
#define GIC_CPU_RUNNINGPRI		0x14
#define GIC_CPU_HIGHPRI			0x18
#define GIC_CPU_ALIAS_BINPOINT		0x1c
#define GIC_CPU_ACTIVEPRIO		0xd0
#define GIC_CPU_IDENT			0xfc

/*GICC_ENABLE: 启用CPU接口

GICC_INT_PRI_THRESHOLD: 中断优先级阈值

GICC_IAR_INT_ID_MASK: 中断ID掩码

GICC_INT_SPURIOUS: 虚假中断ID值

GICC_DIS_BYPASS_MASK: 禁用旁路掩码*/
#define GICC_ENABLE			0x1
#define GICC_INT_PRI_THRESHOLD		0xf0
#define GICC_IAR_INT_ID_MASK		0x3ff
#define GICC_INT_SPURIOUS		1023
#define GICC_DIS_BYPASS_MASK		0x1e0




/*这些寄存器用于全局中断管理，偏移地址从分发器基地址开始：

GIC_DIST_CTRL (0x000): 分发器控制寄存器

GIC_DIST_CTR (0x004): 控制器类型寄存器

GIC_DIST_IGROUP (0x080): 中断分组寄存器

GIC_DIST_ENABLE_SET (0x100): 中断使能设置寄存器

GIC_DIST_ENABLE_CLEAR (0x180): 中断使能清除寄存器

GIC_DIST_PENDING_SET (0x200): 挂起中断设置寄存器

GIC_DIST_PENDING_CLEAR (0x280): 挂起中断清除寄存器

GIC_DIST_ACTIVE_SET (0x300): 活动中断设置寄存器

GIC_DIST_ACTIVE_CLEAR (0x380): 活动中断清除寄存器

GIC_DIST_PRI (0x400): 中断优先级寄存器

GIC_DIST_TARGET (0x800): 中断目标CPU寄存器

GIC_DIST_CONFIG (0xc00): 中断配置寄存器

GIC_DIST_SOFTINT (0xf00): 软件生成中断寄存器

GIC_DIST_SGI_PENDING_CLEAR (0xf10): SGI挂起清除寄存器

GIC_DIST_SGI_PENDING_SET (0xf20): SGI挂起设置寄存器*/
#define GIC_DIST_CTRL			0x000
#define GIC_DIST_CTR			0x004
#define GIC_DIST_IGROUP			0x080
#define GIC_DIST_ENABLE_SET		0x100
#define GIC_DIST_ENABLE_CLEAR		0x180
#define GIC_DIST_PENDING_SET		0x200
#define GIC_DIST_PENDING_CLEAR		0x280
#define GIC_DIST_ACTIVE_SET		0x300
#define GIC_DIST_ACTIVE_CLEAR		0x380
#define GIC_DIST_PRI			0x400
#define GIC_DIST_TARGET			0x800
#define GIC_DIST_CONFIG			0xc00
#define GIC_DIST_SOFTINT		0xf00
#define GIC_DIST_SGI_PENDING_CLEAR	0xf10
#define GIC_DIST_SGI_PENDING_SET	0xf20

/*这些寄存器用于全局中断管理，偏移地址从分发器基地址开始：

GIC_DIST_CTRL (0x000): 分发器控制寄存器

GIC_DIST_CTR (0x004): 控制器类型寄存器

GIC_DIST_IGROUP (0x080): 中断分组寄存器

GIC_DIST_ENABLE_SET (0x100): 中断使能设置寄存器

GIC_DIST_ENABLE_CLEAR (0x180): 中断使能清除寄存器

GIC_DIST_PENDING_SET (0x200): 挂起中断设置寄存器

GIC_DIST_PENDING_CLEAR (0x280): 挂起中断清除寄存器

GIC_DIST_ACTIVE_SET (0x300): 活动中断设置寄存器

GIC_DIST_ACTIVE_CLEAR (0x380): 活动中断清除寄存器

GIC_DIST_PRI (0x400): 中断优先级寄存器

GIC_DIST_TARGET (0x800): 中断目标CPU寄存器

GIC_DIST_CONFIG (0xc00): 中断配置寄存器

GIC_DIST_SOFTINT (0xf00): 软件生成中断寄存器

GIC_DIST_SGI_PENDING_CLEAR (0xf10): SGI挂起清除寄存器

GIC_DIST_SGI_PENDING_SET (0xf20): SGI挂起设置寄存器*/
#define GICD_ENABLE			0x1
#define GICD_DISABLE			0x0
#define GICD_INT_ACTLOW_LVLTRIG		0x0
#define GICD_INT_EN_CLR_X32		0xffffffff
#define GICD_INT_EN_SET_SGI		0x0000ffff
#define GICD_INT_EN_CLR_PPI		0xffff0000
#define GICD_INT_DEF_PRI		0xa0
#define GICD_INT_DEF_PRI_X4		((GICD_INT_DEF_PRI << 24) |\
					(GICD_INT_DEF_PRI << 16) |\
					(GICD_INT_DEF_PRI << 8) |\
					GICD_INT_DEF_PRI)



/*GICH_HCR (0x0): 虚拟化控制寄存器

GICH_VTR (0x4): 虚拟化类型寄存器

GICH_VMCR (0x8): 虚拟机控制寄存器

GICH_MISR (0x10): 维护中断状态寄存器

GICH_EISR0 (0x20): 结束中断状态寄存器0

GICH_EISR1 (0x24): 结束中断状态寄存器1

GICH_ELRSR0 (0x30): 空列表寄存器状态0

GICH_ELRSR1 (0x34): 空列表寄存器状态1

GICH_APR (0xf0): 活动优先级寄存器

GICH_LR0 (0x100): 列表寄存器0起始地址*/
#define GICH_HCR			0x0
#define GICH_VTR			0x4
#define GICH_VMCR			0x8
#define GICH_MISR			0x10
#define GICH_EISR0 			0x20
#define GICH_EISR1 			0x24
#define GICH_ELRSR0 			0x30
#define GICH_ELRSR1 			0x34
#define GICH_APR			0xf0
#define GICH_LR0			0x100


/*GICH_HCR_EN: 启用虚拟化接口

GICH_HCR_UIE: 启用下溢中断

GICH_LR_*: 列表寄存器相关标志

GICH_VMCR_*: 虚拟机控制寄存器相关字段

GICH_MISR_*: 维护中断状态标志*/
#define GICH_HCR_EN			(1 << 0)
#define GICH_HCR_UIE			(1 << 1)

#define GICH_LR_VIRTUALID		(0x3ff << 0)
#define GICH_LR_PHYSID_CPUID_SHIFT	(10)
#define GICH_LR_PHYSID_CPUID		(7 << GICH_LR_PHYSID_CPUID_SHIFT)
#define GICH_LR_STATE			(3 << 28)
#define GICH_LR_PENDING_BIT		(1 << 28)
#define GICH_LR_ACTIVE_BIT		(1 << 29)
#define GICH_LR_EOI			(1 << 19)

#define GICH_VMCR_CTRL_SHIFT		0
#define GICH_VMCR_CTRL_MASK		(0x21f << GICH_VMCR_CTRL_SHIFT)
#define GICH_VMCR_PRIMASK_SHIFT		27
#define GICH_VMCR_PRIMASK_MASK		(0x1f << GICH_VMCR_PRIMASK_SHIFT)
#define GICH_VMCR_BINPOINT_SHIFT	21
#define GICH_VMCR_BINPOINT_MASK		(0x7 << GICH_VMCR_BINPOINT_SHIFT)
#define GICH_VMCR_ALIAS_BINPOINT_SHIFT	18
#define GICH_VMCR_ALIAS_BINPOINT_MASK	(0x7 << GICH_VMCR_ALIAS_BINPOINT_SHIFT)

#define GICH_MISR_EOI			(1 << 0)
#define GICH_MISR_U			(1 << 1)

int gic_init(int chip, unsigned long dist_base, unsigned long cpu_base);
void gic_handle_irq(void);
void gicv2_unmask_irq(int irq);
void gicv2_eoi_irq(int irq);

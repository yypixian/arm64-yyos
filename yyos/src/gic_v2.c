#include "arm-gic.h"
#include "io.h"
#include "asm/irq.h"
#include "printk.h"
#include "timer.h"

struct gic_chip_data
{
    unsigned long raw_dist_base;//dist仲裁寄存器地址
    unsigned long raw_cpu_base;//cpu_interface寄存器地址
    struct irq_domain *domain;//irq_domain是Linux内核中用于管理硬件中断HW_IRQ
    // 和Linux虚拟中断号（VIRQ）映射的核心数据结构
    struct irq_chip *chip;//irq_chip结构体定义了中断控制器的底层操作函数集
    unsigned int gic_irqs;//中断源个数
};


#define gic_dist_base(d) ((d)->raw_dist_base)
#define gic_cpu_base(d) ((d)->raw_cpu_base)

/*对于每一个核，定义一个GIC控制器的data*/
#define ARM_GIC_MAX_NR 1
static struct gic_chip_data gic_data[ARM_GIC_MAX_NR];

static unsigned long gic_get_dist_base(void)
{
	struct gic_chip_data *gic = &gic_data[0];

	return gic_dist_base(gic);
}


static unsigned long gic_get_cpu_base(void)
{
	struct gic_chip_data *gic = &gic_data[0];

	return gic_cpu_base(gic);
}


/*设置某一个中断源*/
static void set_irq(int irq, unsigned long offset)
{
    unsigned int mask = 1 << (irq % 32);// 计算该中断在32位组中的位置
    writel(mask, gic_get_dist_base() + offset + (irq / 32)*4);
    //irq / 32: 计算属于哪一组寄存器
}


/*对于某一个中断源，屏蔽它的中断*/
void gicv2_mask_irq(int irq)
{
    set_irq(irq,GIC_DIST_ENABLE_CLEAR);
}

/*对于某一个中断源，启动它的中断*/
void gicv2_unmask_irq(int irq)
{
    set_irq(irq,GIC_DIST_ENABLE_SET);
}

//写入eoi寄存器，报告中断完成
void gicv2_eoi_irq(int irq)
{
	writel(irq, gic_get_cpu_base() + GIC_CPU_EOI);
}

/*GIC_DIST_TARGETn（Interrupt Processor Targets Registers）
位于Distributor HW block中，
能控制送达的CPU interface*/
static unsigned int gic_get_cpumask(struct gic_chip_data *gic)
{
    unsigned long base = gic_dist_base(gic);
    unsigned int i, mask;

    for(i=mask=0;i<32;i+=4)
    {
        /*mask读取的是32bit的值，将这个值压缩到最低8位，来表示SGI和PPI支持哪些CPU*/
        mask = readl(base + GIC_DIST_TARGET + i);
        mask |= mask >> 16;
        mask |= mask >> 8;
        /*由于不同GIC实现可能使用不同的GIC_DIST_TARGET寄存器来存储掩码，
        代码会遍历前8个寄存器(0-31号中断对应的寄存器)直到找到非零值。*/
        if(mask)
        {
            break;
        }
    }
    return mask;
}

void gic_handle_irq(void)
{
    struct gic_chip_data *gic = &gic_data[0];
	unsigned long base = gic_cpu_base(gic);
	unsigned int irqstat, irqnr;

    do{
        irqstat = readl(base + GIC_CPU_INTACK);//读这个NAK寄存器开始处理中断
        irqnr = irqstat & GICC_IAR_INT_ID_MASK;//取出这个域,此时发生中断的中断号

        printk("%s: irqnr %d\n", __func__, irqnr);
        if (irqnr == GENERIC_TIMER_IRQ)
        {
            handle_timer_irq();
        }
        else if(irqnr == SYSTEM_TIMER1_IRQ)
        {
            handle_stimer_irq();
        }
        gicv2_eoi_irq(irqnr);//中断处理完成
    }while(0);

}



void gic_dist_init(struct gic_chip_data *gic)
{
    unsigned long base = gic_dist_base(gic);
    unsigned int cpu_mask;
    unsigned int gic_irqs = gic->gic_irqs;
    int i;

    /* 关闭中断*/
    writel(GICD_DISABLE, base + GIC_DIST_CTR);

    /* 设置中断路由：GIC_DIST_TARGET
	 * 每个SPI中断都需要明确指定它可以发送到哪些CPU核心
        这就是GIC_DIST_TARGET寄存器的作用
	 * 前32个中断怎么路由是GIC芯片固定的，因此先读GIC_DIST_TARGET前面的值
	 * 然后全部填充到 SPI的中断号 */
    cpu_mask = gic_get_cpumask(gic);
    /*将其扩至32位，用于后续的SPI中断GIC_DIST_TARGET的路由，默认跟前32位路由相同*/
    cpu_mask |= cpu_mask << 8;
    cpu_mask |= cpu_mask << 16;

    /*写入*/
    for(i=32;i<gic_irqs;i+=4)
    {
        writel(cpu_mask, base + GIC_DIST_TARGET + i* 4 / 4);
    }

    /* Set all global interrupts to be level triggered, active low */
    for(i=32;i<gic_irqs;i+=16)
    {
        writel(GICD_INT_ACTLOW_LVLTRIG, base + GIC_DIST_CONFIG + i / 4);
    }

    /* Deactivate and disable all 中断（SGI， PPI， SPI）.
	 *
	 * 当注册中断的时候才 enable某个一个SPI中断，例如调用gic_unmask_irq()*/
    for(i=32;i<gic_irqs;i+=32)
    {
        writel(GICD_INT_EN_CLR_X32, base +
            GIC_DIST_ACTIVE_CLEAR + i / 8);// 活动中断清除寄存器
        writel(GICD_INT_EN_CLR_X32, base +
            GIC_DIST_ENABLE_CLEAR + i / 8);//中断使能清除寄存器
    }

    /*打开SGI中断（0～15），可能SMP会用到*/
	writel(GICD_INT_EN_SET_SGI, base + GIC_DIST_ENABLE_SET);

	/* 打开中断：Enable group0 and group1 interrupt forwarding.*/
	writel(GICD_ENABLE, base + GIC_DIST_CTRL);
}

static void gic_cpu_init(struct gic_chip_data *gic)
{
    int i;
	unsigned long base = gic_cpu_base(gic);
	unsigned long dist_base = gic_dist_base(gic);

    /*
	 * Set priority on PPI and SGI interrupts
	*/
    for(i=0;i<32;i+=4)
    {
        /*每个优先级寄存器控制4个中断的优先级(每个中断8位)
            0xa0a0a0a0为4个中断同时设置优先级0xa0
            GIC优先级数值越小优先级越高(0最高，0xff最低)
            0xa0是一个中等优先级值
            循环为0-31号中断设置相同的优先级*/
        writel(0xa0a0a0,dist_base + GIC_DIST_PRI + i * 4 / 4);
    }
    /*GIC_CPU_PRIMASK是优先级掩码寄存器
    设置一个优先级阈值，只有优先级高于此值的中断才会被CPU接口转发给CPU
    GICC_INT_PRI_THRESHOLD通常定义为0xf0，表示优先级值小于0xf0的中断才会被处理*/
    writel(GICC_INT_PRI_THRESHOLD, base + GIC_CPU_PRIMASK);


    //使能CPU接口
    writel(GICC_ENABLE, base + GIC_CPU_CTRL);

}


int gic_init(int chip, unsigned long dist_base, unsigned long cpu_base)
{
    struct gic_chip_data *gic;
    int gic_irqs;
    //int virq_base;//虚拟地址

    gic = &gic_data[chip];//对于这个核心分配一个gic
    gic->raw_dist_base = dist_base;
    gic->raw_cpu_base = cpu_base;

    /* readout how many interrupts are supported*/
	/*计算当前的仲裁器支持多少的中断源*/
    /*& 0x1f操作提取寄存器的低5位（bit[4:0]），这对应于GIC架构规范中的ITLinesNumber字段*/
    gic_irqs = readl(gic_dist_base(gic) + GIC_DIST_CTR) & 0x1f;
    gic_irqs = (gic_irqs + 1) *32;//这5位表示的是支持的最大中断号除以32再减1（这是GIC规范定义的计算方式
    //最大只能支持1020个中断源
    if (gic_irqs > 1020)
    {
        gic_irqs = 1020;
    }

    gic->gic_irqs = gic_irqs;

    printk("%s: cpu_base:0x%x, dist_base:0x%x, gic_irqs:%d",
        __func__, cpu_base, dist_base, gic->gic_irqs);
    
    gic_dist_init(gic);//初始化dist
	gic_cpu_init(gic);//初始化cpu_interface
    
    return 0;
}











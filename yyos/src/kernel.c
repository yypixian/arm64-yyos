#include "uart.h"
#include "printk.h"
#include "arm-gic.h"
#include "asm/base.h"
#include "timer.h"
#include "irq.h"
#include "mmu.h"
#include "page_alloc.h"
#include "mm.h"


extern void test_S(void);
extern void memcopy_S(void);
extern void LDR_ADR_test(void);

//每一个段的内存布局
extern char text_boot_start[], text_boot_end[];
extern char text_start[], text_end[];
extern char rodata_start[], rodata_end[];
extern char data_start[], data_end[];
extern char bss_begin[], bss_end[];

extern unsigned long atomic_write(unsigned long data);
extern void atomic_set(int data, unsigned long *addr);

extern void trigger_alignment(void);


/*实现拷贝32字节*/
static void my_memory_copy(unsigned long src, unsigned long dst,
					unsigned long counter)
{
	unsigned long tmp = 0;
	unsigned long end = src + counter;

	asm volatile (
		"1: ldr %0, [%1], #8\n"
		"str %0, [%2], #8\n"
		"cmp %1, %3\n"
		"b.cc 1b"
		: "+r" (tmp), "+r" (src), "+r" (dst)
		: "r" (end)
		: "memory");
}
/*
void my_ldr_str_test(void)
{

	memset(0x200004, 0x55, 102);

}
*/

void print_memory(void)
{
	printk("text_boot: 0x%08lx - 0x%08lx (%dB)\n",
		(unsigned long)text_boot_start,
		(unsigned long)text_boot_end,
		(int)(text_boot_end-text_boot_start));
	printk("text: 0x%08lx - 0x%08lx (%dB)\n",
		(unsigned long)text_start,
		(unsigned long)text_end,
		(int)(text_end-text_start));
	printk("rodata: 0x%08lx - 0x%08lx (%dB)\n",
		(unsigned long)rodata_start,
		(unsigned long)rodata_end,
		(int)(rodata_end-rodata_start));
	printk("data: 0x%08lx - 0x%08lx (%dB)\n",
		(unsigned long)data_start,
		(unsigned long)data_end,
		(int)(data_end-data_start));
	printk("bss: 0x%08lx - 0x%08lx (%dB)\n",
		(unsigned long)bss_begin,
		(unsigned long)bss_end,
		(int)(bss_end-bss_begin));
}

static int test_access_map_address(void)
{
	unsigned long address = (512 * 0x100000) - 4096;

	*(unsigned long *)address = 0x55;

	printk("%s access 0x%x done\n", __func__, address);

	return 0;
}

/*
 * 访问一个没有建立映射的地址
 * 应该会触发一级页表访问错误。
 *
 * Translation fault, level 1
 *
 * 见armv8.6手册第2995页
 */
static int test_access_unmap_address(void)
{
	unsigned long address = (512 * 0x100000) + 4096;

	*(unsigned long *)address = 0x55;

	printk("%s access 0x%x done\n", __func__, address);

	return 0;
}


/**/
static void test_mmu(void)
{
	test_access_map_address();
	test_access_unmap_address();
}


void kernel_main(void)
{
	//unsigned long ret;
	uart_init();

	init_printk_done();
	printk("printk init done\n");

	

	//LDR_ADR_test();
	//ret = atomic_write(0x34);
	//printk("arfter write is 0x%x\n",ret);

	//unsigned long p2 = 0;
	
	//atomic_set(0x11, &p2);
	//printk("arfter set is 0x%x\n",p2);

	print_memory();

	printk("Welcome BenOS!\r\n");

	//gic_init(0,GIC_V2_DISTRIBUTOR_BASE,GIC_V2_CPU_INTERFACE_BASE);

	//timer_init();//ARM端的定时器初始化
	//system_timer_init();//树莓派的系统定时器的初始化
	//raw_local_irq_enable();//开启arm的DIAF字段


	
	paging_init();//开启mmu映射


	dump_pgtable();
	//test_mmu();
	//trigger_alignment();
	while (1) {
		uart_send(uart_recv());
	}
}

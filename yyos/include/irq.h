
/*这段代码是用来控制 
ARM处理器 的 DAIF (Exception Mask Bits) 寄存器中的 IRQ (中断请求) 屏蔽位 的，*/

/*msr daifclr, #2：清除 DAIF 寄存器的 bit 1（IRQ 位）。
daifclr 表示 "Clear DAIF bits"，#2 是二进制10（bit1置1，表示要清除 IRQ 屏蔽位）。
"memory" 告诉编译器内存可能被修改，防止优化导致指令重排。*/
static inline void arch_local_irq_enable(void)
{
	asm volatile(
		"msr	daifclr, #2"
		:
		:
		: "memory");
}


/*msr daifset, #2：设置 DAIF 寄存器的 bit 1（IRQ 位）。
daifset 表示 "Set DAIF bits"，#2 是二进制 10（bit 1 置 1，表示要屏蔽 IRQ）。
"memory" 同样用于防止编译器优化。*/
static inline void arch_local_irq_disable(void)
{
	asm volatile(
		"msr	daifset, #2"
		:
		:
		: "memory");
}


#define raw_local_irq_disable()	arch_local_irq_disable()
#define raw_local_irq_enable()	arch_local_irq_enable()


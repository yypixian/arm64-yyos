#include <asm/irq.h>
#include "arm-gic.h"
#include <io.h>
#include <asm/arm_local_reg.h>
#include "printk.h"

void irq_handle(void)
{
#if 0
	unsigned int irq = readl(ARM_LOCAL_IRQ_SOURCE0);

	switch (irq) {
	case (CNT_PNS_IRQ):
		handle_timer_irq();
		break;
	default:
		printk("Unknown pending irq: %x\r\n", irq);
	}
#endif
	gic_handle_irq();

}

#include "asm/pl_uart.h"
#include "asm/gpio.h"

.align 2
.global __init_uart
.global put_uart
__init_uart:
    /*// clean gpio14 
	selector = readl(GPFSEL1);
	selector &= ~(7<<12);
	// set alt0 for gpio14 
	selector |= 4<<12;
	// clean gpio15 
	selector &= ~(7<<15);
	// set alt0 for gpio15 
	selector |= 4<<15;
	writel(selector, GPFSEL1); */
    ldr x1, =GPFSEL1
    ldr w0, [x1]
    /*把这个GPFSEL1寄存器的地址放入w032位通用寄存器中做计算，
    因为树莓派很多寄存器都是32位的 */

    and w0, w0, #0xffff8fff /* selector &= ~(7<<12) */
    orr w0, w0, #0x4000 /* selector |= 4<<12; */
	and w0, w0, #0xfffc7fff  /* selector &= ~(7<<15);*/
	orr w0, w0, #0x20000 /* selector |= 4<<15;*/
	str w0, [x1]

/* 兼容3b*/
#ifdef CONFIG_BOARD_PI3B
	ldr x1, =GPPUD
	str wzr,[x1]

	/* delay */
	mov x0, #150
1:
	sub x0, x0, #1
	cmp x0, #0
	bne 1b

	ldr x1, =GPPUDCLK0
	ldr w2, #0xc000
	str w2, [x1]

	/* delay */
	mov x0, #150
2:
	sub x0, x0, #1
	cmp x0, #0
	bne 2b

	ldr x1, =GPPUDCLK0
	str wzr, [x1]

	isb
#endif

    /* Disable UART */
	ldr x1, =U_CR_REG
	str wzr, [x1]//写0关闭

    /* set BRD */
	ldr x1, =U_IBRD_REG
	mov w2, #26
	str w2, [x1]

    ldr x1, =U_FBRD_REG
	mov w2, #3
	str w2, [x1]
    
    ldr x1, =U_LCRH_REG
	mov w2, #0x70  //(1<<4) | (3<<5)
	str w2, [x1]

	ldr x1, =U_IMSC_REG
	str wzr, [x1]

	ldr x1, =U_CR_REG
	mov w2, #0x301 //1 | (1<<8) | (1<<9)
	str w2, [x1]

	isb//同步
	ret

put_uart:
    ldr x1, =U_FR_REG
1:
    ldr w2, [x1]
    and w2, w2, #0x20
    cmp w2, #0x0
    b.ne 1b
    /*wait for transmit FIFO to have an available slot*/

    ldr x1, =U_DATA_REG
    str w0, [x1]//用32位寄存器，
    //将48+N的ASCII码的值写入数据发送寄存器
    ret

.global put_string_uart
put_string_uart:
    mov x4, x0
	/* save lr register */
	mov x6, x30
2:
    ldrb w0, [x4]//一个一个字节的加载
    bl put_uart
    add x4, x4, 1
    cmp w0, #0
    b.ne 2b

    /* restore lr and return*/
	mov x30, x6
    ret

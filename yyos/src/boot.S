#include "mm.h"
#include "sysregs.h"

//首先定义一个字符串，
//并且链接的时候把这个字符串放到只读数据段里面
.section .rodata
.align 3
.global el_string
el_string:
	.string "Booting at EL"

.section ".text.boot"

.global _start
_start:
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, master		// Hang for all non-primary CPU
	b	proc_hang

proc_hang: 
	b 	proc_hang

master:
        /* init uart and print the string*/
	bl __init_uart

	mrs x5, CurrentEL
	cmp x5, #CurrentEL_EL3
	b.eq el3_entry
	b el2_entry


el3_entry:
	eret

el2_entry:
	bl print_el

	/* The Execution state for EL1 is AArch64 */
	ldr x0, =HCR_HOST_NVHE_FLAGS
	msr hcr_el2, x0

	ldr x0, =SCTLR_VALUE_MMU_DISABLED
	msr sctlr_el1, x0

	ldr x0, =SPSR_EL1
	msr spsr_el2, x0

	adr x0, el1_entry
	msr elr_el2, x0

	eret

el1_entry:
	bl print_el


	/* 设置异常向量表基地址到vbar寄存器 */
	ldr     x5, =vectors
	msr     vbar_el1, x5
	isb

	adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl 	memzero

	



	mov	sp, #LOW_MEMORY 
	bl	kernel_main
	b 	proc_hang		// should never come here
print_el:
	mov x10, x30//保存返回地址

	/*
	print EL
	*/
	adrp x0, el_string//用adrp指令，四字节对齐，防止触发未对齐异常
	add x0, x0, :lo12:el_string
	/*因为adrp会4字节对齐，所以加上它的低12位偏移得到完整的地址*/
	bl put_string_uart//将x0字符串的地址作为参数传入打印函数	


	mrs x5, CurrentEL
	/* get the currentEL value  */
	lsr x2, x5, #2//逻辑右移两位,提取 EL 编号（1/2/3）
	mov x0, #48//将立即数 48（ASCII 字符 '0' 的编码）存入 x0
	add x0, x0, x2//得到此时异常等级编号的字符ASCII码
	bl put_uart//调用函数打印

	/* print the new line tab */
	mov x0, #10
	bl put_uart

	mov x30, x10
	ret
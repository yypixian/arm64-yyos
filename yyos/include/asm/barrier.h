
/*这段代码定义了 ARM64 架构下的内存屏障指令（Memory Barriers），
用于控制 CPU 和内存之间的执行顺序和可见性*/
#ifndef ASM_BARRIER_H
#define ASM_BARRIER_H

#ifndef __ASSEMBLY__
#define isb()		asm volatile("isb" : : : "memory")
//刷新处理器流水线，确保所有后续指令从缓存或内存中重新读取
#define dmb(opt)	asm volatile("dmb " #opt : : : "memory")
//确保内存访问顺序，但不会阻塞指令执行

#define dsb(opt)	asm volatile("dsb " #opt : : : "memory")
//确保内存访问顺序，但不会阻塞指令执行
#endif

#endif /*ASM_BARRIER_H*/

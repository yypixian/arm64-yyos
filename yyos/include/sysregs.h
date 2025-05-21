#ifndef	_SYSREGS_H
#define	_SYSREGS_H

/*用来设置SCTLR_EL1寄存器，打开MMU*/
#define SCTLR_ELx_M	(1<<0)


/*设置HCR寄存器，用来配置高异常等级转化异常等级，处理器的执行环境*/
#define HCR_RW (1 << 31)
#define HCR_HOST_NVHE_FLAGS  (HCR_RW)

/*设置SCTLR寄存器，设置EL0和EL1模式的大小端和关闭MMU*/
#define SCTLR_EE_LITTLE_ENDIAN (0 << 25)
#define SCTLR_EOE_LITTLE_ENDIAN (0 << 24)
#define SCTLR_MMU_DISABLE (0 << 0)
#define SCTLR_VALUE_MMU_DISABLED (SCTLR_EE_LITTLE_ENDIAN | SCTLR_EOE_LITTLE_ENDIAN | SCTLR_MMU_DISABLE)


/*SPSR寄存器屏蔽位，DAIF分别位一些中断需要关闭，（BIT6~BIT9位）*/
#define SPSR_MASK_ALL (7 << 6)
//defne SPSR_MASK (1<<6 | 1<< 7 | 1<< 8 | 1<<9)
#define SPSR_EL1h (5 << 0)//M域中EL1模式，异常返回后是这个模式
#define SPSR_EL2h (9 << 0)//M域中EL2模式，异常返回后是这个模式
#define SPSR_EL1 (SPSR_MASK_ALL | SPSR_EL1h)
#define SPSR_EL2 (SPSR_MASK_ALL | SPSR_EL2h)


#define CurrentEL_EL1 (1 << 2)
#define CurrentEL_EL2 (2 << 2)
#define CurrentEL_EL3 (3 << 2)

#endif
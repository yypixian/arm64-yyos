#ifndef	_RW_REGS_H
#define	_RW_REGS_H

/*
 * 在带参数的宏，#号作为一个预处理运算符,
 * 可以把记号转换成字符串
 *
 * 下面这句话会在预编译阶段变成：
 *  asm volatile("mrs %0, " "reg" : "=r" (__val)); __val; });
 */
 #define read_sysreg(reg) ({ \
    unsigned long _val; \
    asm volatile("mrs %0," #reg \
    : "=r"(_val)); \
    _val; \
})

#define write_sysreg(val, reg) ({ \
    unsigned long _val = (unsigned long)val; \
    asm volatile("msr " #reg ", %x0" \
    :: "rZ"(_val)); \
})


#endif
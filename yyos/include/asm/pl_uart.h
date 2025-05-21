#ifndef	_P_UART_H
#define	_P_UART_H

#include "base.h"

#define U_BASE       (PBASE+0x00201000)

#define U_DATA_REG   (U_BASE)//数据寄存器
#define U_FR_REG     (U_BASE+0x18)//标志寄存器
#define U_IBRD_REG   (U_BASE+0x24)//波特率除数
#define U_FBRD_REG   (U_BASE+0x28)//小数波特率除数
#define U_LCRH_REG   (U_BASE+0x2C)//线路控制寄存器
#define U_CR_REG     (U_BASE+0x30)//控制寄存器
#define U_IMSC_REG   (U_BASE+0x38)//屏蔽设置清除寄存器

#endif  /*_P_UART_H */

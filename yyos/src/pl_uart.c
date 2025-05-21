#include "asm/pl_uart.h"
#include "asm/gpio.h"
#include "io.h"



/*
U_FR_REG 是UART的标志寄存器（Flag Register），
其中第5位（TXFF，Transmit FIFO Full）表示发送FIFO是否已满。
如果 TXFF 为1，则表示发送FIFO已满，需要等待；否则继续执行。
while (readl(U_FR_REG) & (1<<5)) 循环检查发送FIFO的状态，直到有空槽为止
*/
void uart_send(char c)
{
	/* wait for transmit FIFO to have an available slot*/
	while (readl(U_FR_REG) & (1<<5))
		;

	writel(c, U_DATA_REG);
}


/*
U_FR_REG 的第4位（RXFE，Receive FIFO Empty）表示接收FIFO是否为空。
如果 RXFE 为1，则表示接收FIFO为空，需要等待；否则继续执行。
while (readl(U_FR_REG) & (1<<4)) 循环检查接收FIFO的状态，直到有数据可读。
使用 readl(U_DATA_REG) 从数据寄存器读取接收到的数据。
& 0xFF 提取低8位，因为UART通常传输的是8位数据。

*/
char uart_recv(void)
{
	/* wait for receive FIFO to have data to read */
	while (readl(U_FR_REG) & (1<<4))
		;

	return(readl(U_DATA_REG) & 0xFF);
}

void uart_send_string(char *str)
{
	int i;

	for (i = 0; str[i] != '\0'; i++)
		uart_send((char) str[i]);
}


void putchar(char c)
{
       if (c == '\n')
               uart_send('\r');
       uart_send(c);
}

/*
GPIO14和GPIO15被复用为UART的TX（发送）和RX（接收）引脚。
GPFSEL1 是GPIO功能选择寄存器，控制GPIO14和GPIO15的功能。
清除相关位（~(7<<12) 和 ~(7<<15)），然后设置为ALT0模式（4<<12 和 4<<15）。
ALT0模式表示这些引脚被分配给UART功能。
*/
void uart_init(void)
{
	unsigned int selector;

	/* clean gpio14 */
	selector = readl(GPFSEL1);
	selector &= ~(7<<12);
	/* set alt0 for gpio14 */
	selector |= 4<<12;
	/* clean gpio15 */
	selector &= ~(7<<15);
	/* set alt0 for gpio15 */
	selector |= 4<<15;
	writel(selector, GPFSEL1);

#ifdef CONFIG_BOARD_PI3B
	writel(0, GPPUD);
	delay(150);
	writel((1<<14) | (1<<15), GPPUDCLK0);
	delay(150);
	writel(0, GPPUDCLK0);
#else
	/*set gpio14/15 pull down state*/
	selector = readl(GPIO_PUP_PDN_CNTRL_REG0);
	selector |= (0x2 << 30) | (0x2 << 28);
	writel(selector, GPIO_PUP_PDN_CNTRL_REG0);	
#endif

	/* disable UART until configuration is done */
	writel(0, U_CR_REG);

	/*
	 * baud divisor = UARTCLK / (16 * baud_rate)
	= 48 * 10^6 / (16 * 115200) = 26.0416666667
	integer part = 26
	fractional part = (int) ((0.0416666667 * 64) + 0.5) = 3
	generated baud rate divisor = 26 + (3 / 64) = 26.046875
	generated baud rate = (48 * 10^6) / (16 * 26.046875) = 115177
	error = |(115177 - 115200) / 115200 * 100| = 0.02%
	*/


	/*波特率计算公式：baud divisor = UARTCLK / (16 * baud_rate)。
	假设UART时钟频率为48 MHz，目标波特率为115200：
	整数部分：48 * 10^6 / (16 * 115200) = 26。
	小数部分：(int)((0.0416666667 * 64) + 0.5) = 3。
	将整数部分写入 U_IBRD_REG，小数部分写入 U_FBRD_REG。*/
	/* baud rate divisor, integer part */
	writel(26, U_IBRD_REG);
	/* baud rate divisor, fractional part */
	writel(3, U_FBRD_REG);


	/*
	U_LCRH_REG 是UART的线路控制寄存器（Line Control Register）。
	设置：
	第4位（FEN，FIFO Enable）为1，启用FIFO。
	第5-6位（WLEN，Word Length）为3，表示8位数据帧。
	*/
	/* enable FIFOs and 8 bits frames */
	writel((1<<4) | (3<<5), U_LCRH_REG);

	/*U_IMSC_REG 是UART的中断屏蔽寄存器（Interrupt Mask Set/Clear Register）。
	写入0屏蔽所有中断*/
	/* mask interupts */
	writel(0, U_IMSC_REG);

	/*
	再次写入 U_CR_REG：
	第0位（UARTEN，UART Enable）为1，启用UART。
	第8位（RXE，Receive Enable）为1，启用接收。
	第9位（TXE，Transmit Enable）为1，启用发送。
	*/
	/* enable UART, receive and transmit */
	writel(1 | (1<<8) | (1<<9), U_CR_REG);
}

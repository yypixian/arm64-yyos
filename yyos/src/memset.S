.global __memset_16bytes
__memset_16bytes:
    //函数调用，函数的三个参数分别传入了x0, x1, x2寄存器 
    mov x4, #0

1:
    stp x1, x1, [x0], #16//#16表示这条指令写16个字节
	add x4, x4, #16
	cmp x4, x2
	bne 1b
	ret



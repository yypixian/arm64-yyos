
#define MY_LABEL 0x20
.global test_S
test_S:
    //mov x1, 0x80000
    //mov x3, 16

    //ldr x0, [x1]

    //ldr x2, [x1, #8]

    //ldr x4, [x1, x3]

    //ldr x5, [x1, x3, lsl #3]

    //ldr x6, [x1, #8]!

    //ldr x7, [x1], #8


    mov x2, 0x400000
    ldr x6, =0x1234abce

    str x6, [x2, #8]!

    mov x2, 0x500000
    str x6, [x2], #8


    ldr x6, MY_LABEL
    ldr x7, =MY_LABEL

    ret

.global memcopy_S
memcopy_S:
    /* 
    mov x1, 0x80000
    ldr x2, [x1]

    ldr x3, [x1, #8]!

    ldr x4, [x1, #8]!

    ldr x5, [x1, #8]!

    mov x6, 0x200000

    str x2, [x6], #8

    str x3, [x6], #8

    str x4, [x6], #8

    str x5, [x6]
    */
    mov x1, 0x80000
    mov x2, 0x200000
    add x3, x1, 32

1:
    ldr x4, [x1], #8
    str x4, [x2], #8
    cmp x1, x3
    b.cc 1b


    ret


.global CMP_test
CMP_test:
    //cmp x0, x1
    //sbc x0, xzr, xzr
    /*测试CMN指令 */
    mov x1, 1
    mov x2, -3
1:
    cmn x1, x2
    add x2, x2, 1
    /*查看NZCV的操作后缀 */
    mrs x0, nzcv

    /*mi  */
    b.mi 1b//跳转指令操作后缀

    /* 测试CMP指令*/
    mov x2, 3

2:
    cmp x2, x1
    add x1, x2, 1

    mrs x0, nzcv

    /*mi  */
    b.cs 2b//跳转指令操作后缀
    ret



.global UBFX_test
UBFX_test:
    //sbfx x4, x2, #4, #8

    /* 使用ubfx指令来读取寄存器的位域*/
    mrs x1, ID_AA64ISAR0_EL1

    /*读取这个寄存器中atmoic域的值，判断是否支持LES指令 */
    ubfx x0, x1, #20, #4

    /*读取这个寄存器中AES域的值，判断是否支持AES指令 */
    ubfx x2, x1, #4, #4
    ret

.global CESL_test
CESL_test:
    cmp x0, 0
    sub x2, x1, 1
    add x3, x1, 3
    csel x0, x3, x2, eq
    ret


.align 3
.global my_test_data
my_test_data:
    .dword 0x12345678abcdabcd

.global LDR_ADR_test
LDR_ADR_test:
    adr x0, my_test_data

    adrp x1, my_test_data
    add x1, x1, #:lo12:my_test_data
    ldr x3, [x1]

    ldr x4, =my_test_data
    ldr x5, my_test_data


    /*分别使用ldr和adr来加载boot.s里面定义的一级页表 */
    	/*分别使用adrp和ldr来加载位于4MB地址处的init_pg_dir*/
	adrp x2, init_pg_dir
	ldr x6, =init_pg_dir
	ret









.align 3
.global my_data
my_data:
    .dword 0x0


.global atomic_write
atomic_write:
    adr x6, my_data

1:
    ldxr x2, [x6]
    orr x2, x2, x0
    stxr w3, x2, [x6]
    cbnz w3, 1b

    mov x2, x0
    ret 

.global atomic_set
atomic_set:
1:   
    ldxr x2, [x1]
    orr x2, x2, x0
    stxr w3, x2, [x1]
    cbnz w3, 1b

    ret 


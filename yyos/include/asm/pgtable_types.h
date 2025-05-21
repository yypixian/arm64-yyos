#ifndef ASM_PGTABLE_TYPES_H
#define ASM_PGTABLE_TYPES_H

#include <asm/type.h>

typedef u64 pteval_t;
typedef u64 pmdval_t;
typedef u64 pudval_t;
typedef u64 pgdval_t;



typedef struct {
    pgdval_t pgd;
} pgd_t;
#define pgd_val(x)  ((x).pgd)//直接找到表项内容
#define __pgd(x)     ((pgd_t)  { (x) })//将原始值（u64）封装成对应的结构体

typedef struct {
    pudval_t pud;
} pud_t;
#define pud_val(x)  ((x).pud)//直接找到表项内容
#define __pud(x)     ((pud_t)  { (x) })//将原始值（u64）封装成对应的结构体

typedef struct {
    pmdval_t pmd;
} pmd_t;
#define pmd_val(x)  ((x).pmd)//直接找到表项内容
#define __pmd(x)     ((pmd_t)  { (x) })//将原始值（u64）封装成对应的结构体

typedef struct {
    pteval_t pte;
} pte_t;
#define pte_val(x)  ((x).pte)//直接找到表项内容
#define __pte(x)     ((pte_t)  { (x) })//将原始值（u64）封装成对应的结构体


#endif
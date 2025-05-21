#ifndef	_MEMSET_H
#define	_MEMSET_H

#include "asm/type.h"


extern void *__memset_16bytes(void *s, unsigned long val, unsigned long count);
void *memset(void *s, int c, size_t count);

#endif




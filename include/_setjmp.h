#ifndef __TL_SETJMP_H
#define	__TL_SETJMP_H

typedef struct
{
/* setjmp() and longjmp() rely on the order of these registers,
so do not re-arrange them */
	unsigned edi, esi, ebp, esp, ebx, edx, ecx, eax;
	unsigned eip, eflags;
} jmp_buf[1];

int setjmp(jmp_buf b);
void longjmp(jmp_buf b, int ret_val);

#endif

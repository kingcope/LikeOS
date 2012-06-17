#include <setjmp.h> /* jmp_buf */
/*****************************************************************************
To use setjmp() and longjmp() for asynchronous (interrupt-driven;
pre-emptive) task-switching, we want to enable interrupts simultaneous
with jumping to the task. In other words, we want the EFLAGS and EIP
registers loaded at the same time.

The only instruction that can do this is IRET, which also loads the CS
register. Changing CS is done in code that uses far pointers, and it's
also done when changing address spaces, and when changing privilege levels.
We're not interested in any of those, so just push the current CS value
on the stack and let IRET use that.

Three distinct stack pointer (ESP) values are used in this routine:
- 'Old' or 'current' stack pointer value, which is discarded by
  this routine (use setjmp() to save it)
- ESP is made to point, briefly, to the jmp_buf struct itself
- 'New' ESP value; stored in jmp_buf.esp

Register values are restored from the jmp_buf as follows:
1. Push jmp_buf.eflags, the current CS value, and jmp_buf.eip
   onto the 'new' stack
2. Make ESP point to the jmp_buf struct itself, then use the POPA
   instruction to pop the 7 general purpose registers (ESP is not
   loaded by POPA). The use of POPA means that registers in the
   jmp_buf MUST be stored in the order that POPA expects.
   (Maybe use MOVs instead, to eliminate this restriction?
   Might have to rewrite entire function in asm, instead of C.)
3. Load ESP with the 'new' stack pointer, from jmp_buf.esp
4. Use IRET to pop EIP, CS, and EFLAGS from the 'new' stack
5. ???
6. Profit!	<--- obligatory Slashdot joke

This code does NOT save the floating-point state of the CPU. Either:
1. Don't use floating point, or
2. Don't use floating point in more than one thread, or
3. Rewrite this code so it DOES save the floating-point state, or
4. Save/restore the floating-point state when entering/leaving
   the kernel (protected OS only)
*****************************************************************************/
void longjmp(jmp_buf buf, int ret_val)
{
	unsigned *esp;

/* make sure return value is not 0 */
	if(ret_val == 0)
		ret_val++;
/* EAX is used for return values, so store it in jmp_buf.EAX */
	buf->eax = ret_val;
/* get ESP for new stack */
	esp = (unsigned *)buf->esp;
/* push EFLAGS on the new stack */
	esp--;
	*esp = buf->eflags;
/* push current CS on the new stack */
	esp--;
	__asm__ __volatile__(
		"mov %%cs,%0\n"
		: "=m"(*esp));
/* push EIP on the new stack */
	esp--;
	*esp = buf->eip;
/* new ESP is 12 bytes lower; update jmp_buf.ESP */
	buf->esp = (unsigned)esp;
/* now, briefly, make the jmp_buf struct our stack */
	__asm__ __volatile__(
		"movl %0,%%esp\n"
/* ESP now points to 8 general-purpose registers stored in jmp_buf
Pop them */
		"popa\n"
/* load new stack pointer from jmp_buf */
		"movl -20(%%esp),%%esp\n"
/* ESP now points to new stack, with the IRET frame (EIP, CS, EFLAGS)
we created just above. Pop these registers: */
		"iret\n"
		:
		: "m"(buf));
}

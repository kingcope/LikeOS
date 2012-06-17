#ifndef __INTERRUPTS_H
#define __INTERRUPTS_H

void initialize_pics();
/* the code for setvect() and getvect() in
KSTART.ASM depends on the layout of this structure */
typedef struct
{
	unsigned access_byte, eip;
} vector_t;

/* IMPORTS
from KSTART.ASM */
void getvect(vector_t *v, unsigned vect_num);
void setvect(vector_t *v, unsigned vect_num);
void interrupt_keyboard();
void interrupt_timer();



void intr_fault_divide();
void intr_fault_bound();
void intr_fault_opcode();
void intr_fault_tss();
void intr_fault_segment();
void intr_fault_stack();
void intr_fault_gpf();
void intr_fault_pf();
#endif

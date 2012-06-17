/*
$$$	Software Multitasking
*/

#include "task.h"
#include "defs.h"

int numprocesses=0;
int pidcnt=0;

struct process_t *next_process;

void schedule() {
	struct process_t *search;
	struct process_t *tmp;
	//static int c=30;
	
	search=masterp;
	
	for (;;) {
		if (search->next == NULL) break;
		search=search->next;
	}
	
	if ((search==masterp) || (search->prev == masterp) || (search->prev == NULL)) return;

	if (search->process_time == 0) {
		search->process_time=1;
	
		next_process=search->prev;
		next_process->next=NULL;
		tmp=masterp->next;
		tmp->prev=search;
		search->next=tmp;
		search->prev=masterp;
		masterp->next = search;

		
		/*char buffer[1024];
		sprintf(buffer, "%s %d-->%s %d", search->process_name, search->esp, next_process->process_name, next_process->esp);
		
		putstringc(buffer,1,c,0,0,0,getlfb());			
		c+=15;
		*/
		asm("sti");
		outportb(0x20, 0x20);
		outportb(0xa0, 0x20);
		
		contextswitch(&search->esp, next_process->esp); /*$$$*/
	} else {
			search->process_time--;	
	}
}

void ProcessCreate(void (*entry)(void), char *process_name) {
	struct process_t *proc=(struct process_t*)kalloc(sizeof(struct process_t));
	struct process_t *tmp;
	static unsigned int kernel_stack_base;
    unsigned int *kstack = (unsigned int *)KSTACK_BASE+kernel_stack_base;
    unsigned int kstack_size = 4096*2;
    unsigned int *kstack_top = kstack + kstack_size / sizeof(unsigned int);
	int i;
	
	if (numprocesses == 0) {
		masterp->next = proc;
		proc->prev = masterp;
		proc->next = NULL;
	} else {
		tmp = masterp->next;
		tmp->prev=proc;	
		proc->next=tmp;		
		masterp->next = proc;
		proc->prev = masterp;
	}
	
	proc->process_time=1;
	proc->pid=++pidcnt;
//	proc->threadfunc = entry;
	
    // set the return address to be the start of the first function
	kstack_top--;
  //  *kstack_top = (unsigned int)threadproxy;
    *kstack_top = (unsigned int)entry;

    // simulate initial popad
	for(i=0; i<8; i++) {
        kstack_top--;
        *kstack_top = 0;
    }
 
	proc->esp=kstack_top;
    kernel_stack_base+=kstack_size;	
	strcpy(proc->process_name, process_name);
	numprocesses++;
}

void init_multitasking() {
	masterp=(struct process_t*)kalloc(sizeof(struct process_t));
	strcpy(masterp->process_name, "MASTER"); /*$$$*/
	masterp->pid=0;
	masterp->next=NULL;
	masterp->prev=NULL;
	masterp->esp=NULL;
}

void interrupt_timer() {
	asm("cli");
	
	if (numprocesses > 1) {
		schedule();
	}
	
	asm("sti");	
	outportb(0x20, 0x20);
	outportb(0xa0, 0x20);
}

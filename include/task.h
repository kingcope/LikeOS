#ifndef __MULTITASKING_H
#define __MULTITASKING_H

#define KSTACK_BASE 0x200000

struct process_t {
	void *esp;	
	char process_name[30];
	unsigned int pid;
	int process_time;
	struct process_t *next;
	struct process_t *prev;
	void *threadfunc;
};

struct process_t *masterp;
void ProcessCreate(void (*entry)(void), char *process_name);

#endif

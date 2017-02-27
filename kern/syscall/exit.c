#include <types.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <synch.h>

#include <kern/unistd.h>

void
sys__exit(int32_t *retval, int errorcode)
{
	(void)retval;
	(void)errorcode;

	/*
	 * Something to think about, how can a child contact the parent to call 
	 * SIGCHILD?
	 */
	
	// proc_destroy(curthread->t_proc);
	
}

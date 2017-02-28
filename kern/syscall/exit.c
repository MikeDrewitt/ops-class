#include <types.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <synch.h>

#include <kern/wait.h>
#include <kern/unistd.h>

void
sys__exit(int32_t *retval, int exitcode)
{
	(void)retval;
	(void)exitcode;
	
	kprintf("leaving exit\n");

	/*
	 * Something to think about, how can a child contact the parent to call 
	 * SIGCHILD?
	 *
	 * exit the current proccess, what needs to be cleaned up? 
	 *
	 * IDEA- add a exit status int to the proc struct that this function can 
	 * alter. It can then be grabbed by waitpid and taken from there.
	 * 
	 * Must use _MKWAIT_EXIT(x) to mark the exitcode
	 */

	// no clue how to use this
	// _MKWAIT_EXIT(exitcode);

	// proc_destroy(curthread->t_proc);
	
}

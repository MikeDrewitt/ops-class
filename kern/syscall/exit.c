#include <types.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <synch.h>
#include <addrspace.h>

#include <kern/wait.h>
#include <kern/unistd.h>

void
sys__exit(int32_t *retval, int exitcode)
{
	(void)retval;
	// (void)exitcode;
		

	// The Lord says to set the exitcode to _MKWAIT_EXIT(_exitcode)
	// exitcode = _MKWAIT_EXIT(exitcode);	
	curproc->exitcode = exitcode;
	curproc->running = false;

	as_deactivate();	

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

	// proc_destroy(curthread->t_proc);
	
}

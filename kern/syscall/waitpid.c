#include <types.h>
#include <spl.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <vfs.h>
#include <syscall.h>
#include <lib.h>

#include <synch.h>

#include <kern/wait.h>

/*
 * Possible use of CV to wait until the pid is exited from.
 *
 * return "encoded exit status" in 2nd param 
 * this is gotten from the exitcode that was passed into the exit call
 *
 * if the PID has already exited then return ie: if it's "has exited" or 
 * "does not exist"
 * 
 * ***note- exit wants you to first use all PIDs before reuse, fix fork ****
 * **note note - not really. PIDs have 3 states, running, has exited, does not
 * exist. A process goes from "has exited" -> "does not exist" when every process
 * expected to collect its exit status with waitpid has already done so.
 *
 * if the process does not exit waitpid fails 
 * if status is NULL it does not reasign that value upon the process exit
 *
 * only the parent can collect the childs remains
 *
 * 
 * something to think about, how to see if process "has existed" maybe -1?
 *
 * How to check if we're the parent?
 * does it matter? Is there any other way to get the childs ID other than calling
 * fork?
 *
 * So if I recall correctly we can ignore the options always setting to to 0,
 * however, it tells whether we're blocking, so ignore or implement?
 */

pid_t
sys_waitpid(int32_t *retval, pid_t pid, int *status, int options) {

	(void)retval;
	(void)pid;
	(void)status;
	(void)options;

	struct cv *wait_on;
	wait_on = cv_create("wait_on");

	// if pid DNE then fail.
	
	/*
	 * if pid alread exited return
	 * ie: if pid_table[pid].pid == -1;? 
	 *
	 * here maybe using pid == -1 to represent exited?
	 */

	lock_acquire(curproc->p_full_lock);

	// wait until 
	while (pid_table[pid].pid != -1) {
		kprintf("waiting till exit\n");
		// fork seems to fall into a trap here
		cv_wait(wait_on, curproc->p_full_lock);
	}

	lock_release(curproc->p_full_lock);

	return 0;
}

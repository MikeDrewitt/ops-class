#include <types.h>
#include <spl.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <vfs.h>
#include <syscall.h>
#include <lib.h>

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
 * something to think about, how to see if process "has existed"
 */

pid_t
sys_waitpid(int32_t *retval, pid_t pid, int *status, int options) {

	(void)retval;
	(void)pid;
	(void)status;
	(void)options;

	struct cv *wait_on;
	wait_on = cv_create("wait_on");

	// the curproc->locks->thread is NULL, how to fix? 
	while (pid_table[pid].pid != -1) {
		cv_wait(wait_on, curproc->p_full_lock);
	}

	return 0;
}

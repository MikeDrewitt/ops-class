#include <types.h>
#include <spl.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <vfs.h>
#include <syscall.h>
#include <lib.h>
#include <copyinout.h>

#include <synch.h>

#include <kern/wait.h>
#include <kern/errno.h>

/*
 * Possible use of CV to wait until the pid is exited from.
 *
 * return "encoded exit status" in 2nd param 
 * this is taken from the exitcode that was passed into the exit call
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
 * something to think about, how to see if process "has existed" maybe -1?
 * or are we supposed to use something like WIFEXITED(x)? or is that for just
 * usercode?
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

	

	// (void)retval;
	// (void)status;
	(void)options;

	// kprintf("WAIT => cur pid: %d\n", curproc->pid);

	// kprintf("WAIT => param pid: %d\n", pid);
	
	// kprintf("WAITING ON => name: %s\n", pid_table[pid]->p_name);
	// kprintf("WAITING ON => pid: %d\n", pid_table[pid]->pid);
	// kprintf("WAITING ON  => Parent pid: %d\n", pid_table[pid]->parent_pid);
	// kprintf("WAITING ON  => running: %d\n", pid_table[pid]->running);
	// kprintf("WAITING ON  => exitcode: %d\n", pid_table[pid]->exitcode);
		
	if (pid < 0 || pid > 64 || pid_table[pid] == NULL) {
		*retval = ESRCH;
		return -1;
	}

	if (status != NULL && ((int)status % 4) != 0) {
		*retval = EFAULT;
		return -1;
	}

	if (curproc->pid == pid) {
		*retval = EFAULT;
		return -1;
	}

	if (options < 0 || options > 2) {
		*retval = EINVAL;
		return -1;
	}

	if (curproc->pid != pid_table[pid]->parent_pid) {
		*retval = ECHILD;
		return -1;
	}

	if (options == WNOHANG) {
		if (pid_table[pid]->p_sem->sem_count == 0) {
			return 0;
		}
	}

	lock_acquire(curproc->p_full_lock);
	
	void *safe_status = NULL;
	int result = copyin((const_userptr_t)status, &safe_status, 4);

	if (result) {
		
		if (status == NULL) {
			P(pid_table[pid]->p_sem);
			
			proc_destroy(pid_table[pid]);

			lock_release(curproc->p_full_lock);

			*retval = pid;
			return 0;
		}
		
		
		lock_release(curproc->p_full_lock);
		*retval = EFAULT;
		return -1;
	}

	// Wait here until _exit() is called by pid
	P(pid_table[pid]->p_sem);

	if (safe_status != NULL) {
		*status = pid_table[pid]->exitcode;
	}

	proc_destroy(pid_table[pid]);

	lock_release(curproc->p_full_lock);

	/*
	 * when done waiting return the exit status from _exit() in *status
	 */

	*retval = pid;
	return 0;
}

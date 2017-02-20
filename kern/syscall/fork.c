
#include <types.h>
#include <spl.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <vfs.h>
#include <syscall.h>
#include <lib.h>

#include <kern/fcntl.h>
pid_t 
sys_fork(int32_t *retval) {
	(void)retval;
	
	/*
	 * Create new proc and copy current 
	 *
	 * set a new pid (must be > 0)
	 * 
	 */

	// kprintf("curproc: %s\n", curproc->p_name);
	// kprintf("0 index: %s\n", pid_table[0].p_name);

	struct proc *child_proc;
	child_proc = create_proc("child_proc");

	struct addrspace *child_addr;
	struct trapframe *child_tf;
	
	//copys trap frame 
	bzero(&child_proc->p_tf, sizeof(child_proc->p_tf));

	child_tf = curproc->p_tf;
	
	// kprintf("cp: %d pp: %d\n", child_tf->tf_t3, curproc->p_tf->tf_t3);

	//copy parent address space into child proc
	as_copy(curproc->p_addrspace, &child_addr);
	child_proc->p_addrspace = child_addr;

	//copy parent thread into parent
	int err = thread_fork("child_thread", child_proc,(void *)child_tf, 0, 0);

	kprintf("err: %d\n", err);

	int i = 0;
	while (i < 64) {
		child_proc->p_filetabel[i] = curproc->p_filetabel[i];
		i++;
	}

	i = 0;
	while (pid_table[i].p_name != NULL) {
		i++; // probably will introduce bug if > 64 processes. 
	}

	pid_table[i] = *child_proc;
	child_proc->pid = i;
	
	return 0;
}


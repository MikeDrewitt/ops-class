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

static 
void
fork_entry(void *data1, unsigned long *data2){
	(struct trapframe)data1;
	(struct addrspace)data2;
	kprintf("in function pointer\n");
	data1->tf_epc += 4;
	data1->tf_v0 = 0;
	data1->tf_a3 = 0;
	as_activate();
	mips_usermode(&data1);

}



pid_t 
sys_fork(int32_t *retval) {
	(void)retval;
	kprintf("in fork\n");	
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
	child_tf = kmalloc(sizeof(struct trapframe));

	//copys trap frame 
	bzero(&child_proc->p_tf, sizeof(child_proc->p_tf));
	memcpy(child_tf, curproc->p_tf, sizeof(struct trapframe));
	child_proc->p_tf = child_tf;
	

	//child_tf = curproc->p_tf;
	
	// kprintf("cp: %d pp: %d\n", child_tf->tf_t3, curproc->p_tf->tf_t3);

	//copy parent address space into child proc
	as_copy(curproc->p_addrspace, &child_addr);
	child_proc->p_addrspace = child_addr;

	//copy parent thread into parent
	(void *)child_tf;
	(unsigned long *)child_addr;
	(*entry)fork_entry(child_tf, child_addr);
	kprintf("before thread fork\n");
	int err = thread_fork("child_thread", child_proc,(void *)child_tf, child_tf, child_addr);
	kprintf("after thread fork\n");
	(void)err;
	//kprintf("err: %d\n", err);

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
	kprintf("%d\n",child_proc->pidi);
	return child_proc->pid;
}

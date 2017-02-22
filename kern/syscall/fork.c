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
fork_entry(void *data1, unsigned long data2){
	
	//curproc->p_tf->tf_v0 = 0;
	//curproc->p_tf->tf_a3 = 0;
	//curproc->p_tf->tf_epc += 4;
	
		
	struct trapframe tf;
/*
	tf.tf_vaddr = curproc->p_tf->tf_vaddr;
	tf.tf_status = curproc->p_tf->tf_status;
	tf.tf_cause = curproc->p_tf->tf_cause;
	tf.tf_lo = curproc->p_tf->tf_lo;
	tf.tf_hi = curproc->p_tf->tf_hi;
	tf.tf_ra = curproc->p_tf->tf_ra;
	tf.tf_at = curproc->p_tf->tf_at;
	tf.tf_v1 = curproc->p_tf->tf_v1;
	tf.tf_a1 = curproc->p_tf->tf_a1;
	tf.tf_a0 = curproc->p_tf->tf_a0;
	tf.tf_a2 = curproc->p_tf->tf_a2;
	tf.tf_t0 = curproc->p_tf->tf_t0;
	tf.tf_t1 = curproc->p_tf->tf_t1;
	tf.tf_t2 = curproc->p_tf->tf_t2;
	tf.tf_t3 = curproc->p_tf->tf_t3;
	tf.tf_t4 = curproc->p_tf->tf_t4;
	tf.tf_t5 = curproc->p_tf->tf_t5;
	tf.tf_t6 = curproc->p_tf->tf_t6;
	tf.tf_t7 = curproc->p_tf->tf_t7;
	tf.tf_s0 = curproc->p_tf->tf_s0;
	tf.tf_s1 = curproc->p_tf->tf_s1;
	tf.tf_s2 = curproc->p_tf->tf_s2;
	tf.tf_s3 = curproc->p_tf->tf_s3;
	tf.tf_s4 = curproc->p_tf->tf_s4;
	tf.tf_s5 = curproc->p_tf->tf_s5;
	tf.tf_s6 = curproc->p_tf->tf_s6;
	tf.tf_s7 = curproc->p_tf->tf_s7;
	tf.tf_s8 = curproc->p_tf->tf_s8;
	tf.tf_t8 = curproc->p_tf->tf_t8;
	tf.tf_t9 = curproc->p_tf->tf_t9;
	tf.tf_gp = curproc->p_tf->tf_gp;
	tf.tf_sp = curproc->p_tf->tf_sp;
	tf.tf_s8 = curproc->p_tf->tf_s8;
*/

	memcpy(&tf, curproc->p_tf, sizeof(*curproc->p_tf));

	tf.tf_v0 = 0;
	tf.tf_a3 = 0;
	tf.tf_epc += 4;

	as_activate();
	mips_usermode(&tf);

	(void)data1;
	(void)data2;

	return;
}

pid_t 
sys_fork(int32_t *retval) {
	(void)retval;
	// kprintf("in fork\n");	
	/*
	 * Create new proc and copy current 
	 *
	 * set a new pid (must be > 0)
	 * 
	 */

	struct proc *child_proc;
	child_proc = create_proc("child_proc");
	
	struct addrspace *child_addr;
	struct trapframe *child_tf;

	child_tf = kmalloc(sizeof(struct trapframe));

	//copys trap frame 
	bzero(child_tf, sizeof(struct trapframe));
	memcpy(child_tf, curproc->p_tf, sizeof(struct trapframe));
	child_proc->p_tf = child_tf;

	//copy parent address space into child proc
	as_copy(curproc->p_addrspace, &child_addr);
	child_proc->p_addrspace = child_addr;

	//copy parent thread into parent

	//	void (*func)(void *, unsigned long *);
	//	func = fork_entry((void *)child_tf, (unsigned long *)child_addr);

	thread_fork("child_thread", child_proc, fork_entry,(void *)child_tf, (unsigned long)child_addr);
	
	// (void)err;
	// kprintf("err: %d\n", err);

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
	
	// kprintf("%d\n",child_proc->pid);

	return child_proc->pid;
}

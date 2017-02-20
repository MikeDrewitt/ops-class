
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

	struct proc *child_proc;
	child_proc = create_proc("child_proc");

	struct addrspace *child_addr;
	struct trapframe *child_tf;
	
	//copys trap frame 
	bzero(&child_proc->p_tf, sizeof(child_proc->p_tf));
/*
	child_tf->tf_vaddr = curproc->p_tf->tf_vaddr;
	child_tf->tf_status = curproc->p_tf->tf_status;
	child_tf->tf_cause = curproc->p_tf->tf_cause;
	child_tf->tf_lo = curproc->p_tf->tf_lo;
	child_tf->tf_hi = curproc->p_tf->tf_hi;
	child_tf->tf_ra = curproc->p_tf->tf_ra;
	child_tf->tf_at = curproc->p_tf->tf_at;
	child_tf->tf_v0 = curproc->p_tf->tf_v0;
	child_tf->tf_v1 = curproc->p_tf->tf_v1;
	child_tf->tf_a0 = curproc->p_tf->tf_a0;
	child_tf->tf_a1 = curproc->p_tf->tf_a1;
	child_tf->tf_a2 = curproc->p_tf->tf_a2;
	child_tf->tf_a3 = curproc->p_tf->tf_a3;
	child_tf->tf_t0 = curproc->p_tf->tf_t0;
	child_tf->tf_t1 = curproc->p_tf->tf_t1;
	child_tf->tf_t2 = curproc->p_tf->tf_t2;
	child_tf->tf_t3 = curproc->p_tf->tf_t3;
	child_tf->tf_t4 = curproc->p_tf->tf_t4;
	child_tf->tf_t5 = curproc->p_tf->tf_t5;
	child_tf->tf_t6 = curproc->p_tf->tf_t6;
	child_tf->tf_t7 = curproc->p_tf->tf_t7;
	child_tf->tf_s0 = curproc->p_tf->tf_s0;
	child_tf->tf_s1 = curproc->p_tf->tf_s1;
	child_tf->tf_s2 = curproc->p_tf->tf_s2;
	child_tf->tf_s3 = curproc->p_tf->tf_s3;
	child_tf->tf_s4 = curproc->p_tf->tf_s4;
	child_tf->tf_s5 = curproc->p_tf->tf_s5;
	child_tf->tf_s6 = curproc->p_tf->tf_s6;
	child_tf->tf_s7 = curproc->p_tf->tf_s7;
	child_tf->tf_t8 = curproc->p_tf->tf_t8;
	child_tf->tf_t9 = curproc->p_tf->tf_t9;
	child_tf->tf_gp = curproc->p_tf->tf_gp;
	child_tf->tf_sp = curproc->p_tf->tf_sp;
	child_tf->tf_s8 = curproc->p_tf->tf_s8;
	child_tf->tf_epc = curproc->p_tf->tf_epc;
	child_proc->p_tf = child_tf;
	//thread_fork("child_proc", child_proc, );
*/

	child_tf = curproc->p_tf;

	//copy parent address space into child proc
	as_copy(curproc->p_addrspace, &child_addr);
	child_proc->p_addrspace = child_addr;

	//copy parent thread into parent
	thread_fork("child_thread", child_proc,(void *)child_tf, (unsigned long *)child_addr, 0);




	if (child_proc == NULL) {
		kprintf("null\n");
	}
	
	 return 0;
}


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
#include <kern/wait.h>
#include <kern/errno.h>

static 
void
fork_entry(struct trapframe *data1, unsigned long data2){
		
	struct trapframe tf;

	memcpy(&tf, data1, sizeof(*data1));

	// kprintf("sizeof trapframe: %d\n", sizeof(*curproc->p_tf));
	// kprintf("trap cause: %x\n", tf.tf_cause);

	// kprintf("CHILD => child PID: %d\n",curproc->pid);
	// kprintf("CHILD => parent PID: %d\n",curproc->parent_pid);
	// kprintf("CHILD => child exitcode: %d\n",curproc->exitcode);
	// kprintf("CHILD => child running bool: %d\n",curproc->running);

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
	// (void)retval;
	// kprintf("in fork\n");	
	/*
	 * Create new proc and copy current 
	 *
	 * set a new pid (must be > 0)
	 * 
	 */

	lock_acquire(curproc->p_full_lock);

	// kprintf("FORK -> My ID: %d\n", curproc->pid);

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

	int i = 0;
	while (i < 64) {
		if (curproc->p_filetable[i] != NULL) {
			curproc->p_filetable[i]->ref_counter += 1;
		}
		
		child_proc->p_filetable[i] = curproc->p_filetable[i];

		// kprintf("child: %p\n", child_proc->p_filetable[i]);
		// kprintf("parent: %p\n", curproc->p_filetable[i]);

		i++;
	}

	// kprintf("child: %p\n", child_proc->p_filetable);
	// kprintf("parent: %p\n", curproc->p_filetable);

	int new_pid = 1;
	while (pid_table[new_pid] != NULL) {
		new_pid++; // probably will introduce bug if > 64 processes. 

		if (new_pid == 64) {
			*retval = ENPROC;
			return -1;
		}
	}

	child_proc->pid = new_pid;
	child_proc->parent_pid = curproc->pid;
	child_proc->exitcode = -1;
	child_proc->running = true;


	pid_table[new_pid] = child_proc;
	
	thread_fork("child_thread", child_proc, (void *)fork_entry, child_tf, (unsigned long)child_addr);

	// kprintf("...\n");

	// kprintf("FORK => numthreads: %d\n", child_proc->p_numthreads);
	// kprintf("FORK => child PID: %d\n",child_proc->pid);
	// kprintf("FORK => parent PID: %d\n",child_proc->parent_pid);
	// kprintf("FORK => child exitcode: %d\n",child_proc->exitcode);
	// kprintf("FORK => child running bool: %d\n",child_proc->running);

	// kprintf("parent cause: %x\n", curproc->p_tf->tf_cause);

	*retval = child_proc->pid;

	lock_release(curproc->p_full_lock);

	return 0;
}

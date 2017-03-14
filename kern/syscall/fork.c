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


	int kill;

	/*
	 * if we never see an open spot we need
	 * to defuse the bomb. 
	 *
	 * There's a bomb when no spots are open
	 * ie: all_indexes != NULL
	 */

	// kprintf("FORK -> My ID: %d\n", curproc->pid);

	int bomb;
	int open_space = 0;
	for (bomb = 0; bomb < PID_TOP; bomb++) {
		if (pid_table[bomb] == NULL) {
			// there's an open space
			open_space = 1;
		}
	}

	if (!open_space) {
		// kprintf("no space\n");
		*retval = ENPROC;
		return -1;
	}

	// LEAKING?
	struct proc *child_proc;
	child_proc = create_proc("child_proc");

	if (child_proc == NULL) {
		*retval = ENOMEM;
		return -1;
	}
	
	struct addrspace *child_addr;
	struct trapframe *child_tf;

	child_tf = kmalloc(sizeof(struct trapframe));

	if (child_tf == NULL) {
		*retval = ENOMEM;
		return -1;
	}

	//copys trap frame 
	bzero(child_tf, sizeof(struct trapframe));
	memcpy(child_tf, curproc->p_tf, sizeof(struct trapframe));
	
	child_proc->p_tf = child_tf;



	//copy parent address space into child proc
	kill = as_copy(curproc->p_addrspace, &child_addr);	
	if (kill) {
		kfree(child_tf);
		*retval = ENOMEM;
		
		return -1;
	}
	
	child_proc->p_addrspace = child_addr;

	int i = 0;
	while (i < PID_TOP) {
		if (curproc->p_filetable[i] != NULL) {

			curproc->p_filetable[i]->ref_counter += 1;
			kprintf("fork ref count: %d    fd: %d    \n", curproc->p_filetable[i]->ref_counter, i);
		}
		
		// kprintf("iter: %d\n", i);
		child_proc->p_filetable[i] = curproc->p_filetable[i];

		// kprintf("child: %p\n", child_proc->p_filetable[i]);
		// kprintf("parent: %p\n", curproc->p_filetable[i]);

		i++;
	}

	// kprintf("child: %p\n", child_proc->p_filetable);
	// kprintf("parent: %p\n", curproc->p_filetable);


	int new_pid = 1;
	while (pid_table[new_pid] != NULL && new_pid < PID_TOP) {
		new_pid++; // probably will introduce bug if > PID_TOP processes. 
	}

	if (pid_table[new_pid] == NULL) {	
		child_proc->pid = new_pid;
		child_proc->parent_pid = curproc->pid;
		child_proc->exitcode = -1;
		child_proc->running = true;

		// kprintf("\n");
		kill = vfs_getcurdir(&child_proc->p_cwd);
		if (kill) {
			//kprintf("vfs_getcwd failed (%s)\n", strerror(kill));
			*retval = kill;
			
			return -1;
		}


		pid_table[new_pid] = child_proc;
	}
	else {
		// panic("You tried to overwrite another proccess.\n");
		// kprintf("No Room!\n");
		*retval = ENPROC;
		
		return -1;
	}
	
	kill = thread_fork("child_thread", child_proc, (void *)fork_entry, child_tf, (unsigned long)child_addr);
	if (kill) {
		kfree(child_tf);
		*retval = ENOMEM;
		
		return -1;
	}

	// kprintf("FORK => numthreads: %d\n", child_proc->p_numthreads);
	// kprintf("FORK => child PID: %d\n",child_proc->pid);
	// kprintf("FORK => parent PID: %d\n",child_proc->parent_pid);
	// kprintf("FORK => child exitcode: %d\n",child_proc->exitcode);
	// kprintf("FORK => child running bool: %d\n",child_proc->running);

	// kprintf("parent cause: %x\n", curproc->p_tf->tf_cause);

	*retval = child_proc->pid;
	// kprintf("%p\n", NULL);


	return 0;
}

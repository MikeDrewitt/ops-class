#include <types.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <syscall.h>
#include <test.h>
#include <copyinout.h>
#include <limits.h>

#include <kern/fcntl.h>
#include <kern/errno.h>

	
int
sys_execv(int32_t *retval, const char *program, char **args)
{

//TODO:
//-instanciate kernel buffer to max_buf_len (64k)                             X
//kernel buffer is a char  **kbuff                                            X
//
//-loop through args and copy all the arguments into kernel buffer 
//-stop looping when you get to null
//-be sure to include padding to make sure the length of the argument is 
//devisible by 4 (add in NULL)
//

	(void)retval;
	
	// kprintf("CALL TO EXEC CONNECTED\n");

	bzero(k_buff, ARG_MAX);
	int arg_start = 0;

	while (*args != NULL) {
		kprintf("arg: %s adr: %p poniter_addr: %p\n", *args, args, &args);
		
		k_buff = *args;

		// copyin((const_userptr_t)args, k_buff, 4);
		kprintf("karg: %s adr: %p pointer_addr: %p\n", (char *)k_buff, k_buff, &k_buff);
		
		//*k_buff = (void *)*args;
		
		k_buff += 4;
		args += 1;
		arg_start += 4;
	}

	args -= (arg_start / 4);
	kprintf("start: %d\n", arg_start);
	
	/* Cleaning out the kernel buffer for this call of EXECV */

	while (*args != NULL) {
		kprintf("args: %s, is at: %p\n", *args, &*args);
	 	
		int padding = 0;

		char *running_arg;
		running_arg = *args;


		while (*running_arg != 0) {
			kprintf("chars: %d\n", *running_arg);
			
			char *cur_char = (char *)k_buff;
			*cur_char = *running_arg;

			padding += 1;
			running_arg += 1;
			k_buff += 1;
			arg_start +=1;
		}

		padding %= 4;

		int i;
		for (i = 0; i < padding; i++) {
			k_buff = '\0';
			k_buff += 1;
			arg_start += 1;
		}

		args++;
	}

	k_buff -= arg_start;

	int i = 0;	
	while (k_buff != NULL) {

		if (i > 16) {
			kprintf("k_buff: %s\n",	(char *)k_buff);
		}

		i += 1;
		k_buff += 1;
	}

	/* BEGIN RUNPROGRAM  */

	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	/* Open the file. */
	char kernel_progname[128];
	copyinstr((const_userptr_t)program, kernel_progname, 128, 0);

	// kprintf("progname: %s\n", kernel_progname);
	result = vfs_open(kernel_progname, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}

	/* We should be a new process. */
	// KASSERT(proc_getas() == NULL);

	/* Create a new address space. */
	as = as_create();
	if (as == NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Switch to it and activate it. */
	proc_setas(as);
	as_activate();

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}

	/* Warp to user mode. */
	enter_new_process(0 /*argc*/, NULL /*userspace addr of argv*/,
			  NULL /*userspace addr of environment*/,
			  stackptr, entrypoint);

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
}

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
//-instanciate kernel buffer to max_buf_len (64k)
//kernel buffer is a char  **kbuff
//
//-loop through args and copy all the arguments into kernel buffer 
//-stop looping when you get to null
//-be sure to include padding to make sure the length of the argument is 
//devisible by 4 (add in NULL)
//
	

	(void)retval;
	// (void)program;
	// (void)args;

	// kprintf("CALL TO EXEC CONNECTED\n");
	
	/* Cleaning out the kernel buffer for this call of EXECV */
	bzero(kern_buff, ARG_MAX);
	int args_start = 0;

	while (*args != NULL) {

		kern_buff[args_start] = *args;

		args_start += 1;
		args += 1;
	}

	args -= args_start;

	while (*args != NULL) {
		char *curstring = *args;
		while (*curstring != 0) {
			kern_buf[args_start] = (char *)*curstring;
			args_start += 1;
			curstring += 1;
		}
	}

	int i = 0;
	while (kern_buff[i] != 0) {
		kprintf("kargs: %s, is %d bytes\n", kern_buff[i], sizeof(kern_buff[i]));
		i += 1;
	}


/*
	while (kern_buff != NULL) {
		kprintf("args: %s, is at: %p\n", kern_buff, &kern_buff);
	 	
		char *running_arg;
		running_arg = kern_buff;
		while (*running_arg != 0) {
			kprintf("chars: %d\n", *running_arg);
			running_arg++;
		}

		kern_buff++;
		args_start++;
		args++;
	}
*/
	
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

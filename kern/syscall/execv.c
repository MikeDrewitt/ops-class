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

	void *buff_top = k_buff;
	char *temp_str[0];
		
	copyin((const_userptr_t)args, (void *)temp_str, 4);

	kprintf("buff pointing to: %p\n", k_buff);
	k_buff = &temp_str[0];

	while (k_buff != NULL) {	

		kprintf("temp: %s\n", temp_str[0]);
	
		k_buff++;
		args++;

		copyin((const_userptr_t)args, temp_str, 4);
		k_buff = (temp_str[0] != NULL) ? &temp_str[0] : NULL;
	}

	kprintf("top pointing to: %p\n", buff_top);
	kprintf("buff pointing to: %p\n", k_buff);

	k_buff = buff_top;
	
	kprintf("top pointing to: %p\n", buff_top);
	kprintf("buff pointing to: %p\n", k_buff);

	kprintf("arg: \'%s\'\n", (char *)k_buff);
	kprintf("point to: \'%p\'\n", k_buff);

	k_buff++;
	kprintf("arg: \'%s\'\n", (char *)k_buff);
	kprintf("point to: \'%p\'\n", k_buff);

	k_buff++;

	kprintf("arg: \'%s\'\n", (char *)k_buff);
	kprintf("point to: \'%p\'\n", k_buff);

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

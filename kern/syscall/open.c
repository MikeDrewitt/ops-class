//#include <unistd.h>
//#include <fcntl.h>

#include <types.h>
#include <syscall.h>
#include <copyinout.h>
#include <vfs.h>
#include <current.h>
#include <proc.h>
#include <synch.h>

#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/fcntl.h>

/*
 * uses vfs open to open the file,
 * gets all of the information from the vnode that is passed in there
 *
 * supposed to return the index in the array for the file descriptor
 *
 * TODO 
 *	error handling
 *	code refractor because Mike can't spell
 *	make sure the function is working properly
 *	free the malloc somewhere . . . close?
 */

int
sys_open(int32_t *retval, const char *filename, int flags)
{
	// (void)retval;
	// (void)filename;
	// (void)flags;


	struct vnode *v;
	struct file_table *file;

	file = kmalloc(sizeof(*file));

	int result;
	char name_copy[128];

	result = copyinstr((const_userptr_t)filename, name_copy, 128, 0);

	if (result) {
		*retval = EFAULT;
		return -1;
	}

	if (filename == NULL) {
		*retval = EFAULT;
		return -1;
	}

	if (*filename == 0) {
		*retval = EINVAL;
		return -1;
	}

	if (flags < 0 || flags > 127) {
		*retval = EINVAL;
		return -1;
	}

	// lock_acquire(curproc->p_full_lock);
	
	file->ft_lock = lock_create("file_lock");
	
	file->flag = flags;
	file->offset = 0;
	file->ref_counter = 1;

	result = vfs_open(name_copy, flags, 0664, &v);		

	if (result) {
		*retval = EIO;
		return -1;
	}

	int file_descriptor = 0; //position in the file table
	while (curthread->t_proc->p_filetable[file_descriptor] != NULL && file_descriptor < 65) {	
		file_descriptor++;
	}//run out of space? 
	
	// kprintf("fd: %d, has been opened\n", file_descriptor);

	file->ft_vnode = v;
	curthread->t_proc->p_filetable[file_descriptor] = file;

	// lock_release(curproc->p_full_lock);

	*retval = file_descriptor;
	return 0;
}

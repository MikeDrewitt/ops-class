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

	if (filename == NULL) {
		*retval = -1;
		return EFAULT;
	}
/*
	if (flags != O_RDONLY && flags != O_WRONLY && flags != O_RDWR) {
		*retval = -1;
		return EINVAL;
	}
*/	
	file->ft_lock = lock_create("file_lock");
	
	file->flag = flags;
	file->offset = 0;

	int result;
	char name_copy[128];


	copyinstr((const_userptr_t)filename, name_copy, 128, 0);
	result = vfs_open(name_copy, flags, 0664, &v);		

	if (result) {
		*retval = -1;
		return EIO;
	}

	int file_descriptor = 0; //position in the file table
	while (curthread->t_proc->p_filetable[file_descriptor] != NULL) {	
		file_descriptor++;
	}//run out of space? 

	file->ft_vnode = v;
	curthread->t_proc->p_filetable[file_descriptor] = file;

	*retval = file_descriptor;
	return 0;
}

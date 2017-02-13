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
 * read syscall 
 */

int
sys_open(int32_t *retval, const char *filename, int flags)
{
	(void)retval;
	// (void)filename;
	// (void)flags;


	struct vnode *v;
	struct file_tabel *file;


	file = kmalloc(sizeof(*file));

	int result;

	if (filename == NULL) {
		//retval = (int32_t)EFAULT;
		return -1;
	}

	if (flags != O_RDONLY && flags != O_WRONLY && flags != O_RDWR) {
		//retval = EINVAL;
		return -1;
	}

	char name_copy[128];

	copyinstr((const_userptr_t)filename, name_copy, 128, 0);
	result = vfs_open(name_copy, flags, 0, &v);		

	
	/*
	 * do actual logic for after open here such as finding a spot to open into
	 * increment the file handler
	 *
	 * iterate over filetable and return a new value that will be the file handle
	 */
	
	int file_descriptor = 0; //position in the file table
	while (curthread->t_proc->p_filetabel[file_descriptor] != NULL) {
		
		file_descriptor++;
	}

	file->ft_vnode = v;
	file->ft_lock = lock_create("file_lock");
	
	file->flag = flags;
	file->offset = 0;

	curproc->p_filetabel[file_descriptor] = file;
	// if true then open worked
	// supposed to return file handle or -1 
	
	return file_descriptor;
}

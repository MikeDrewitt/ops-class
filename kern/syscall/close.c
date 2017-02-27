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
sys_close(int32_t *retval, int fd)
{
	if (fd < 0 || curthread->t_proc->p_filetable[fd] == NULL) {
		*retval = -1;
		return EBADF;
	}
	
	lock_destroy(curthread->t_proc->p_filetable[fd]->ft_lock);

	curthread->t_proc->p_filetable[fd]->flag = 0;
	curthread->t_proc->p_filetable[fd]->offset = 0;

	kfree(curthread->t_proc->p_filetable[fd]);
	curthread->t_proc->p_filetable[fd] = NULL;

	*retval = 0;
	return 0;
}

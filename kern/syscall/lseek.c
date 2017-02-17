
#include <types.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <lib.h>

#include <kern/seek.h>
#include <kern/errno.h>

off_t 
sys_lseek(int32_t *retval, int fd, off_t pos, int whence)
{
	/*
	 *	update the offset position based on the whence 
	 *	of the fd that's passed in.
	 */
	if (curthread->t_proc->p_filetabel[fd] == NULL) {
		*retval = -1;
		return EBADF;
	}

	if (pos < 0) {
		*retval = -1;
		return EINVAL;
	}

	if (whence == SEEK_SET) curthread->t_proc->p_filetabel[fd]->offset = pos;
	else if (whence == SEEK_CUR) curthread->t_proc->p_filetabel[fd]->offset += pos;	
	else if (whence == SEEK_END) {
		// pos + length of file
		panic("WHERE'S THE END?!?!");
		return -1;
	} else {
		*retval = -1;
		return EINVAL;
	}
	return -1;
}


#include <types.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <lib.h>
#include <vnode.h>

#include <kern/stat.h>
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

	struct stat statbuf;
	
	int result;
	result = vop_stat(curthread->t_proc->p_filetabel[fd]->ft_vnode, statbuf);

	(void)result;
	
	if (whence == SEEK_SET) curthread->t_proc->p_filetabel[fd]->offset = pos;
	else if (whence == SEEK_CUR) curthread->t_proc->p_filetabel[fd]->offset += pos;	
	else if (whence == SEEK_END) curthread->t_proc->p_filetabel[fd]->offset = statbuf->st_size + pos;
	else {
		*retval = -1;
		return EINVAL;
	}
	return 0;
}


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
sys_lseek(int64_t *retval, int fd, off_t pos, int whence)
{
	/*
	 *	update the offset position based on the whence 
	 *	of the fd that's passed in.
	 */
	

	if (fd < 0 || fd > 64 || curthread->t_proc->p_filetable[fd] == NULL) {
		*retval = EBADF;
		return -1;
	}

	int seekable = VOP_ISSEEKABLE(curproc->p_filetable[fd]->ft_vnode); 
	if (!seekable) {
		*retval = ESPIPE;
		return -1;
	}

	struct stat statbuf;

	lock_acquire(curproc->p_filetable[fd]->ft_lock);

	VOP_STAT(curproc->p_filetable[fd]->ft_vnode, &statbuf);

	if (whence == SEEK_SET) {
		
		if (pos < 0) {
			*retval = EINVAL;

			lock_release(curproc->p_filetable[fd]->ft_lock);
			return -1;

		}

		curproc->p_filetable[fd]->offset = pos;
		*retval = curthread->t_proc->p_filetable[fd]->offset;
		
		lock_release(curproc->p_filetable[fd]->ft_lock);
		return 0;
	}
	else if (whence == SEEK_CUR) {
	
		int position = pos + curproc->p_filetable[fd]->offset;

		// Seeking beyond the EOF
			
		if (position < 0) {
			*retval = EINVAL;

			lock_release(curproc->p_filetable[fd]->ft_lock);
			return -1;			
		}

		curproc->p_filetable[fd]->offset += pos;
		*retval = curproc->p_filetable[fd]->offset;
		
		lock_release(curproc->p_filetable[fd]->ft_lock);
		return 0;
	}
	else if (whence == SEEK_END) {

		int position = statbuf.st_size + pos;

		if (position < 0) {
			*retval = EINVAL;

			lock_release(curproc->p_filetable[fd]->ft_lock);
			return -1;			
		}

		curproc->p_filetable[fd]->offset = position;
		*retval = curproc->p_filetable[fd]->offset;
		
		lock_release(curproc->p_filetable[fd]->ft_lock);
		return 0; 

	}
	
	*retval = EINVAL;
	lock_release(curproc->p_filetable[fd]->ft_lock);

	return -1;
}

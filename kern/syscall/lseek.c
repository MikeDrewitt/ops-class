
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
	if (curthread->t_proc->p_filetabel[fd] == NULL) {
		*retval = -1;
		return EBADF;
	}

	if (pos < 0) {
		*retval = -1;
		return EINVAL;
	}

	struct stat statbuf;
	
	VOP_STAT(curproc->p_filetabel[fd]->ft_vnode, &statbuf);
/*
	kprintf("#----------------------#\n");
	kprintf("fd: %d\n", fd);
	kprintf("pos_lseek: %lld\n", pos);
	kprintf("whence_lseek: %d\n", whence);
	kprintf("offset_lseek: %d\n", curthread->t_proc->p_filetabel[fd]->offset);
	kprintf("file_size: %lld\n", statbuf.st_size);
	kprintf("SEEK: %d\n", whence);
	kprintf("#----------------------#\n");
*/
	if (whence == SEEK_SET) {
		curproc->p_filetabel[fd]->offset = pos;
		*retval = curthread->t_proc->p_filetabel[fd]->offset;
		return 0;
	}
	else if (whence == SEEK_CUR) {
		curproc->p_filetabel[fd]->offset += pos;
		*retval = curproc->p_filetabel[fd]->offset;
		return 0;
	}
	else if (whence == SEEK_END) {
		curproc->p_filetabel[fd]->offset = statbuf.st_size + pos;
		*retval = curproc->p_filetabel[fd]->offset;
		return 0; 

	}
	
	*retval = -1;
	return EINVAL;
}

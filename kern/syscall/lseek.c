
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

	if (pos < 0) {
		*retval = EINVAL;
		return -1;
	}

	struct stat statbuf;
	
	VOP_STAT(curproc->p_filetable[fd]->ft_vnode, &statbuf);
/*
	kprintf("#----------------------#\n");
	kprintf("fd: %d\n", fd);
	kprintf("pos_lseek: %lld\n", pos);
	kprintf("whence_lseek: %d\n", whence);
	kprintf("offset_lseek: %d\n", curthread->t_proc->p_filetable[fd]->offset);
	kprintf("file_size: %lld\n", statbuf.st_size);
	kprintf("SEEK: %d\n", whence);
	kprintf("#----------------------#\n");
*/
	if (whence == SEEK_SET) {
		curproc->p_filetable[fd]->offset = pos;
		*retval = curthread->t_proc->p_filetable[fd]->offset;
		return 0;
	}
	else if (whence == SEEK_CUR) {
		curproc->p_filetable[fd]->offset += pos;
		*retval = curproc->p_filetable[fd]->offset;
		return 0;
	}
	else if (whence == SEEK_END) {
		curproc->p_filetable[fd]->offset = statbuf.st_size + pos;
		*retval = curproc->p_filetable[fd]->offset;
		return 0; 

	}
	
	*retval = EINVAL;
	return -1;
}

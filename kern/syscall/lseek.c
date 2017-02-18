
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
	
	int result;
	result = VOP_STAT(curthread->t_proc->p_filetabel[fd]->ft_vnode, &statbuf);

	(void)result;

	kprintf("pos: %lld\n", pos);
	kprintf("whence: %d\n", whence);
	kprintf("offset: %d\n", curthread->t_proc->p_filetabel[fd]->offset);

	if (whence == SEEK_SET) {
		curthread->t_proc->p_filetabel[fd]->offset = pos;

		*retval = curthread->t_proc->p_filetabel[fd]->offset;
		return 0;
	}
	else if (whence == SEEK_CUR) {
		curthread->t_proc->p_filetabel[fd]->offset += pos;

		*retval = curthread->t_proc->p_filetabel[fd]->offset;
		return 0;
	}
	else if (whence == SEEK_END) {
		curthread->t_proc->p_filetabel[fd]->offset = statbuf.st_size + pos;
		
		kprintf("stat_size: %llu\n", statbuf.st_size);
		kprintf("new_pos: %u\n", curthread->t_proc->p_filetabel[fd]->offset);

		*retval = curthread->t_proc->p_filetabel[fd]->offset;
		return 0; 

	}
	
	*retval = -1;
	return EINVAL;
}

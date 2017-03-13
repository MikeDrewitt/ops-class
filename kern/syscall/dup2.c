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


int
sys_dup2(int32_t *retval, int oldfd, int newfd)
{
	// (void)retval;
	// (void)oldfd;
	// (void)newfd;

	if (newfd > 64 || newfd < 0 || curproc->p_filetable[oldfd] == NULL) {
		*retval = EBADF;
		return -1;
	}
	
	// If the file at that descriptor was alread open, close the file
	if (curproc->p_filetable[newfd] != NULL) {
	
		lock_acquire(curproc->p_filetable[newfd]->ft_lock);

		if (newfd < 0 || newfd > 64) {
			*retval = EBADF;
			return -1;
		}
			
		if (curthread->t_proc->p_filetable[newfd] == NULL) {
			*retval = EBADF;
			return -1;
		}

		curproc->p_filetable[newfd]->ref_counter--;
		if (curproc->p_filetable[newfd]->ref_counter == 0) {
	
			curthread->t_proc->p_filetable[newfd]->flag = 0;
			curthread->t_proc->p_filetable[newfd]->offset = 0;

			vfs_close(curproc->p_filetable[newfd]->ft_vnode);

			lock_release(curproc->p_filetable[newfd]->ft_lock);
			
			kfree(curthread->t_proc->p_filetable[newfd]);
			curthread->t_proc->p_filetable[newfd] = NULL;

			lock_destroy(curproc->p_filetable[newfd]->ft_lock);
		}
		else lock_release(curproc->p_filetable[newfd]->ft_lock);
	}	
	
	// kprintf("\n** oldfd: %d, newfd %d\n", oldfd, newfd);

	// kprintf("new file lock name: %s\n", curproc->p_filetable[newfd]->ft_lock->lk_name);
	// kprintf("old file lock name: %s\n", curproc->p_filetable[oldfd]->ft_lock->lk_name);
	
	// kprintf("offset 1: %d 2: %d\n", curproc->p_filetable[oldfd]->offset, curproc->p_filetable[newfd]->offset);

	lock_acquire(curproc->p_filetable[oldfd]->ft_lock);

	curproc->p_filetable[newfd] = curproc->p_filetable[oldfd];	// copy over the pointer to the file
	curproc->p_filetable[oldfd]->ref_counter += 1;				// increase times this file is open

	// kprintf("offset 1: %d 2: %d\n", curproc->p_filetable[oldfd]->offset, curproc->p_filetable[newfd]->offset);
	
	lock_release(curproc->p_filetable[oldfd]->ft_lock);

	*retval = newfd;
	return 0;
}

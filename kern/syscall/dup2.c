#include <types.h>
#include <proc.h>
#include <syscall.h>
#include <current.h>

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
	
	lock_acquire(curproc->p_filetable[oldfd]->ft_lock);

	// If the file at that descriptor was alread open, close the file
	if (curproc->p_filetable[newfd] != NULL) {
		int err = sys_close(retval, newfd);
		if (err) {
			lock_release(curproc->p_filetable[oldfd]->ft_lock);
			
			return -1;
		}
	}	
		
	
	kprintf("\n** oldfd: %d, newfd %d\n", oldfd, newfd);

	kprintf("new file lock name: %s\n", curproc->p_filetable[newfd]->ft_lock->lk_name);
	kprintf("old file lock name: %s\n", curproc->p_filetable[oldfd]->ft_lock->lk_name);
	
	kprintf("offset 1: %d 2: %d\n", curproc->p_filetable[oldfd]->offset, curproc->p_filetable[newfd]->offset);

	curproc->p_filetable[oldfd]->ref_counter += 1;				// increase times this file is open
	curproc->p_filetable[newfd] = curproc->p_filetable[oldfd];	// copy over the pointer to the file

	// kprintf("offset 1: %d 2: %d\n", curproc->p_filetable[oldfd]->offset, curproc->p_filetable[newfd]->offset);
	
	lock_release(curproc->p_filetable[oldfd]->ft_lock);

	// kprintf("** oldfd: %d, newfd: %d\n", oldfd, newfd);

	*retval = newfd;
	return 0;
}

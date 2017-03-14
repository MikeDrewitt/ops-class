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

//	(void)oldfd;

	
	// If The newfd is already open
	lock_acquire(curproc->p_filetable[oldfd]->ft_lock);
	if (curproc->p_filetable[newfd] != NULL) {
		// kprintf("dup2 calling close \n");
		sys_close(retval, newfd);
	}
	
	curproc->p_filetable[oldfd]->ref_counter += 1;
	
	// kprintf("dup2 ref count increase = %d \n", curproc->p_filetable[oldfd]->ref_counter);
	
	curproc->p_filetable[newfd] = curproc->p_filetable[oldfd];
	
	lock_release(curproc->p_filetable[oldfd]->ft_lock);
	
	*retval = newfd;
	// kprintf("finishing dup2\n");
	return 0;
}

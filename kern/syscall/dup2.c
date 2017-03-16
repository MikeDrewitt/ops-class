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
	int err;
	
	if(oldfd < 0 || newfd < 0 || oldfd >63 || newfd > 63 || curproc->p_filetable[oldfd] == NULL){
		*retval = EBADF;
		return -1;
	}

	if(oldfd == newfd){
		*retval = oldfd;
		return 0;
	}
	



	lock_acquire(curproc->p_filetable[oldfd]->ft_lock);
	// If The newfd is already open
//	kprintf("file %d has acquired a lock in dup2\n",oldfd);
	if (curproc->p_filetable[newfd] != NULL) {
//		 kprintf("dup2 calling close in\n");
		err = sys_close(retval, newfd);
		if(err){
			
			lock_release(curproc->p_filetable[oldfd]->ft_lock);
			*retval = err;
			return -1;
	
		}
	}
	
	curproc->p_filetable[oldfd]->ref_counter += 1;
	
	// kprintf("dup2 ref count increase = %d \n", curproc->p_filetable[oldfd]->ref_counter);
	
	curproc->p_filetable[newfd] = curproc->p_filetable[oldfd];
	
	 lock_release(curproc->p_filetable[oldfd]->ft_lock);
	
//	kprintf("file %d has released a lock in dup2\n",oldfd);
	*retval = newfd;
	// kprintf("finishing dup2\n");
	return 0;
}

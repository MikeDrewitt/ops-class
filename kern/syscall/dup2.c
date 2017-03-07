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

	if (curproc->p_filetable[oldfd] == NULL || newfd > 64 || newfd < 0) {
		*retval = EBADF;
		return -1;
	}

	/* If the file at that descriptor was alread open close the file  */
	if (curproc->p_filetable[newfd] != NULL) {
		int err = sys_close(retval, newfd);
		if (err) {
			return -1;
		}
	}

	/* Points to the same location  */
	curproc->p_filetable[newfd] = curproc->p_filetable[oldfd];

	kprintf("offset 1: %d 2: %d\n", curproc->p_filetable[oldfd]->offset, curproc->p_filetable[newfd]->offset);

	kprintf("end dup2\n");
	*retval = newfd;
	return 0;
}

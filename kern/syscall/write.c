//#include <unistd.h>

#include <types.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <addrspace.h>
#include <vnode.h>
#include <elf.h>
#include <copyinout.h>

#include <kern/errno.h>

/*
 * read syscall 
 */
ssize_t
sys_write(int fd, const void *buf, size_t nbytes, int32_t *retval)
{

	/*
	 * we need to write the stuff at buf into the file descriptor
	 * that is passed in as fd. 
	 */


	int result;

	// EBADF fd is not a valid file descriptor, or was not opened for writing 
	if (curproc->p_filetable[fd] == NULL) {
		*retval = -1;
		return EBADF;
	}

	struct iovec iov;
	struct uio u;

	lock_acquire(curproc->p_filetable[fd]->ft_lock);

	// kprintf("%s\n",(char *)buf);

	iov.iov_ubase = (userptr_t)buf;
	iov.iov_len = nbytes;
	u.uio_iov = &iov;
	u.uio_iovcnt = 1;
	u.uio_resid = nbytes;
	u.uio_offset = curproc->p_filetable[fd]->offset;
	u.uio_segflg = UIO_USERSPACE;
	u.uio_rw = UIO_WRITE;
	u.uio_space = curthread->t_proc->p_addrspace;//doesnt seem right

	result = VOP_WRITE(curthread->t_proc->p_filetable[fd]->ft_vnode, &u);

	*retval = nbytes; // - u.uio_resid;
	curproc->p_filetable[fd]->offset += nbytes - u.uio_resid;
	
	lock_release(curproc->p_filetable[fd]->ft_lock);

	if(result){
		*retval = -1;
		return EIO;
	}
// u.uio_resid is updated based on how many bites are written during 
// VOP_write

	return 0;
}

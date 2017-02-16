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
sys_write(int32_t *retval, int fd, const void *buf, size_t nbytes)
{
	(void)retval;	
	// (void)fd;	
	// (void)buf;
	// (void)nbytes;

	/*
	 * we need to write the stuff at buf into the file descriptor
	 * that is passed in as fd. 
	 */
	int result;

	if (curthread->t_proc->p_filetabel[fd] == NULL) {
		// retval = EBADF;
		return -1;
	}

	
	struct iovec iov;
	struct uio u;	

	char buf_copy[64];
	copyinstr((const_userptr_t)buf, buf_copy, 64, 0);

	iov.iov_ubase = (userptr_t)buf_copy;
	u.uio_iov = &iov;
	u.uio_iovcnt = 1;
	u.uio_resid = nbytes;
	u.uio_offset = curthread->t_proc->p_filetabel[fd]->offset;
	u.uio_segflg = UIO_USERSPACE;
	u.uio_rw = UIO_WRITE;
	u.uio_space = curthread->t_proc->p_addrspace;

	result = VOP_WRITE(curthread->t_proc->p_filetabel[fd]->ft_vnode, &u);

	return result ? (size_t)-1 : nbytes;	
}

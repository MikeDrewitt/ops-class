#include <types.h>
#include <copyinout.h>
#include <syscall.h>

/*
 *
 */

size_t
read(int fd, void *buf, size_t buflen);
{
	// implement write
	//
	// kprint(buf);

	struct iovec iov;
	struct uio u;

	iov.iov_ubase = (userptr_t)&buf;
	iov.iov_len = buflen;				// length of the memory space
	
	u.uio_iov = &iov;
	u.uio_iovcnt = 1;
	u.uio_resid = buflen;				// amount to read from the file
	u.uio_offset = 0;
	u.uio_segflg = UIO_USERSPACE;
	u.uio_rw = UIO_READ;
	u.uio_space = curproc->p_filetabel[fd]->fd_addrspace;

	result = VOP_READ(curproc->p_filetabel[fd]->fd_node, &u);
	if (result) {
		return result;
	}

	return 0;
}

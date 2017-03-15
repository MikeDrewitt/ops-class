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
#include <kern/unistd.h>

/*
 * read syscall 
 */
ssize_t
sys_read(int32_t *retval, int fd, void *buf, size_t buflen)
{
	int result;

	// EBADF fd is not a valid file descriptor, or was not opened for writing 
	
	if (fd < 0 || fd > 64 || curthread->t_proc->p_filetable[fd] == NULL) {
		*retval = EBADF;
		return -1;
	}

	KASSERT(buf != NULL);

	/*
	void *safe_buf = NULL;
	result = copyin((const_userptr_t)buf, &safe_buf, buflen);

	if (result) {
		*retval = EFAULT;
		return -1;
	}

	if (safe_buf == NULL) {
		*retval = EFAULT;
		return -1;
	}
	*/

	struct iovec iov;
	struct uio u;

	lock_acquire(curproc->p_filetable[fd]->ft_lock);

	iov.iov_ubase = (userptr_t)buf;
	iov.iov_len = buflen;
	u.uio_iov = &iov;
	u.uio_iovcnt = 1;
	u.uio_resid = buflen;
	u.uio_offset = curproc->p_filetable[fd]->offset;
	u.uio_segflg = UIO_USERSPACE;
	u.uio_rw = UIO_READ;
	u.uio_space = curproc->p_addrspace;//doesnt seem right

	result = VOP_READ(curproc->p_filetable[fd]->ft_vnode, &u);

	if(result){
		lock_release(curproc->p_filetable[fd]->ft_lock);

		*retval = EIO;
		return -1;
	}
// u.uio_resid is updated based on how many bites are written during 
// VOP_write
	*retval = buflen; //- u.uio_resid;
	curproc->p_filetable[fd]->offset += buflen - u.uio_resid;

	lock_release(curproc->p_filetable[fd]->ft_lock);

	return 0;

}

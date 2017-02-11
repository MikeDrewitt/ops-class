//#include <unistd.h>
//#include <fcntl.h>

#include <types.h>
#include <syscall.h>
#include <copyinout.h>

#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/fcntl.h>

/*
 * read syscall 
 */

int
sys_open(int32_t *retval, const char *filename, int flags)
{
	(void)retval;
	(void)filename;
	(void)flags;

	struct vnode *v;

	int result;

	if (filename == NULL) {
		return EFAULT;
	}

	if (flags != O_RDONLY && flags != O_WRONLY && flags != O_RDWR) {
		return EINVAL;
	}

	char *name_copy;

	copyinstr(*filename, name_copy,  (size_t)sizeof(name_copy), 0);
	result = vfs_open(name_copy, flags, 0, &v);		

	if (result) {
		return result;
	}
	
	return 0;
	
}

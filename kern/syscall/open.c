//#include <unistd.h>
//#include <fcntl.h>

#include <types.h>
#include <syscall.h>
#include <copyinout.h>
#include <vfs.h>

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
	// (void)filename;
	// (void)flags;

	struct vnode *v;

	int result;

	if (filename == NULL) {
		//retval = (int32_t)EFAULT;
		return -1;
	}

	if (flags != O_RDONLY && flags != O_WRONLY && flags != O_RDWR) {
		//retval = EINVAL;
		return -1;
	}

	// Are we supposed to declare this with a static size, brijesh says so . . .
	char *name_copy = kmalloc(sizeof(char) * 128);

	copyinstr((const_userptr_t)filename, name_copy, (size_t)sizeof(name_copy), 0);
	result = vfs_open(name_copy, flags, 0, &v);		

	/*
	 * do actual logic for after open here such as finding a spot to open into
	 * increment the file handler
	 */
	
	kfree(name_copy);

	// supposed to return file handle or -1 
	if (result) {
		return result;
	}

	return 0;
	
}

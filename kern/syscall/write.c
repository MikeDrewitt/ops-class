//#include <unistd.h>

#include <types.h>
#include <syscall.h>
#include <kern/unistd.h>

/*
 * read syscall 
 */
ssize_t
sys_write(int32_t *retval, int fd, const void *buf, size_t nbytes)
{
	(void)retval;	
	(void)fd;	
	(void)buf;
	(void)nbytes;
	
	//kprintf("%s", (char*)buf);

	return 0;
}

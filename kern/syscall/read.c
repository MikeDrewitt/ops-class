//#include <unistd.h>

#include <types.h>
#include <syscall.h>
#include <kern/unistd.h>

/*
 * read syscall 
 */
ssize_t
sys_read(int32_t *retval, int fd, void *buf, size_t buflen)
{
	(void)retval;	
	(void)fd;	
	(void)buf;
	(void)buflen;

	return 0;
}

//#include <unistd.h>

#include <types.h>
#include <syscall.h>


/*
 * read syscall 
 */
ssize_t
sys_read(int fd, void *buf, size_t buflen)
{
	(void)fd;	
	(void)buf;
	(void)buflen;

	return 0;
}

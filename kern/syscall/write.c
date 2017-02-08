#include <types.h>
#include <copyinout.h>
#include <syscall.h>

/*
 * Example system call: get the time of day.
 */
size_t
sys_write(int fd, const void *buf, size_t nbytes)
{
	// implement write
	//
	
	(void)fd;
	(void)buf;
	(void)nbytes;

	return 0;
}

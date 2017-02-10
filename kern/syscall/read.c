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

	(void)fd;
	(void)buf;
	(void)buflen;


	return 0;
}

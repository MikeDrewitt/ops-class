//#include <unistd.h>
//#include <fcntl.h>

#include <types.h>
#include <syscall.h>

/*
 * read syscall 
 */

int
sys_open(const char *filename, int flags)
{
	(void)filename;
	(void)flags;
	
	return 0;
}

//#include <unistd.h>
//#include <fcntl.h>

#include <types.h>
#include <syscall.h>

/*
 * read syscall 
 */

int
sys_open(int32_t *retval, const char *filename, int flags)
{
	(void) retval;
	(void)filename;
	(void)flags;
	
	return 0;
}

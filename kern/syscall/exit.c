#include <types.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <synch.h>

#include <kern/unistd.h>

void
sys__exit(int32_t *retval, int errorcode)
{
	(void)retval;
	(void)errorcode;
}

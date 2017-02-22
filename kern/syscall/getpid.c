#include <types.h>
#include <proc.h>
#include <syscall.h>
#include <current.h>

#include <kern/unistd.h>

pid_t
sys_getpid(int32_t *retval)
{
	(void)retval;

	return curproc->pid;
}

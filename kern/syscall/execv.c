#include <types.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <synch.h>
#include <addrspace.h>

#include <kern/wait.h>
#include <kern/unistd.h>

int
sys_execv(int32_t *retval, const char *program, char **args)
{
	(void)retval;
	(void)program;
	(void)args;

	return 0;			
}

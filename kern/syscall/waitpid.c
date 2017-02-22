#include <types.h>
#include <spl.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <vfs.h>
#include <syscall.h>
#include <lib.h>

#include <kern/fcntl.h>

pid_t
sys_waitpid(int32_t *retval, pid_t pid, int *status, int options) {

	(void)retval;
	(void)pid;
	(void)status;
	(void)options;
	
	return 0;
}

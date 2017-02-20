
#include <types.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <lib.h>
#include <vnode.h>

#include <kern/errno.h>

pid_t 
sys_fork(int32_t *retval) {
	(void)retval;
	
	/*
	 * Create new proc and copy current 
	 *
	 * set a new pid (must be > 0)
	 * 
	 */
	
	 return 0;
}


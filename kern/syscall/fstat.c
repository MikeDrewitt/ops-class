#include <types.h>
#include <syscall.h>
#include <copyinout.h>
#include <vfs.h>
#include <current.h>
#include <proc.h>
#include <synch.h>

#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/fcntl.h>

int
sys_fstat(int32_t *retval, int fd, struct stat *statbuf) 
{
	(void)retval;
	(void)fd;
	(void)statbuf;

	return 0;
}



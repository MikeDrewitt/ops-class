#include <types.h>
#include <copyinout.h>
#include <syscall.h>

/*
 * write writes up to buflen bytes to the file specified by fd, at the 
 * location in the file specified by the current seek position of the 
 * file, taking the data from the space pointed to by buf. The file must 
 * be open for writing.
 *
 * The current seek position of the file is advanced by the number of 
 * bytes written.
 *
 * Each write (or read) operation is atomic relative to other I/O to the same 
 * file. Note that the kernel is not obliged to (and generally cannot) make 
 * the write atomic with respect to other threads in the same process 
 * accessing the I/O buffer during the write.
 *
 */

size_t
sys_write(int fd, const void *buf, size_t nbytes)
{
	// implement write
	//
	// kprint(buf);

	(void)fd;
	(void)buf;
	(void)nbytes;


	return 0;
}

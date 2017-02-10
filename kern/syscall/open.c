#include <unistd.h>
#include <fcntl.h>

/*
 * write writes up to buflen bytes to the file specified by fd, 
 * at the location in the file specified by the current seek position 
 * of the file, taking the data from the space pointed to by buf. The 
 * file must be open for writing.
 *
 * The current seek position of the file is advanced by the number of 
 * bytes written.
 *
 * Each write (or read) operation is atomic relative to other I/O to 
 * the same file. Note that the kernel is not obliged to (and generally 
 * cannot) make the write atomic with respect to other threads in the 
 * same process accessing the I/O buffer during the write.
 */

int
sys_open(const char *filename, int flags)
{
// implement open
	//
	
	(void)filename;
	(void)flags;
	int result;
	struct vnode *v;
	
	// Filename invalid pointer
	if (filename == NULL) {
		return EFAULT;
	}

	// Non-valid flags
	if (flags != O_RDONLY && flags != O_WRONGLY && flags != O_RDWR) {
		return EINVAL;
	}

	int i = 0;
	while (curproc->p_filetabel[i] != NULL) {
		i++;
	}

	// i is now the location that we can open new data as that memory is null

	/* 
	switch(flags) {
		case O_RDONLY:
			result = vfs_open(filename, flags, 0, &v);
			if(result){
				return result;
				//some error
			}
			// readonly
			break;
		case O_WRONLY:
			result = vfs_open(filename, flags, 0, &v);
			if(result){
				return result;	//some error
			}
			// write only
			break;
		case O_RDWR:
			result = vfs_open(filename, flags, 0, &v);
			if(result){
				return result;
			}

			// both
			break;	
		default:
			panic("Not proper flags.");	
	}
	*/

	result = vfs_open(filename, flags, 0, &v);
	if(result){
		return result;
	}

	return 0;
}

/*
int
sys_open(const char *filename, int flags, mode_t mode)
{
	(void)filename;
	(void)flags;
	(void)mode;

	return 0;

}
*/

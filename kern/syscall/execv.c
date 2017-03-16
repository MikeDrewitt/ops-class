#include <types.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <syscall.h>
#include <test.h>
#include <copyinout.h>
#include <limits.h>

#include <kern/fcntl.h>
#include <kern/errno.h>

int
sys_execv(int32_t *retval, const char *program, char **args)
{


	//(void)retval;

	int err;

	void *k_buff = carl_k_buff; 
	bzero(k_buff, ARG_MAX);

	/*
	 * pointer_void - pointer to the kernel buffer that will be used to assign 
	 *				  the pointers to the arguments.
	 *
	 * top_buff - the pointer to the kernel buffer that is used for copying 
	 *            from the kernel buffer to the user stack.
	 *
	 * pointer_char - pointer to the kernel that is used for copying in the
	 *                arguments from userspace.
	 *
	 * safe_args - safe version of the args variable being passed in
	 *
	 */

	void *pointer_void = k_buff;
	void *top_buff = k_buff;
	char *pointer_char = (char *)k_buff;
//	char **safe_args = k_buff;

	/*
	 * num_args - number of arguments being passed in.
	 *
	 * buffer_offset - total number of bytes of all the pointers including
	 *                 NULL.
	 *
	 * arg_buff_count - total number of bytes the arguments + the padding
	 *                  takes up.
	 *
	 * buff_size - total number of bytes used in the kernel buffer.
	 *
	 * if_first - flag to tell if the current character is the first character
	 *            in the argument
	 *
	 * cur_arg - shows numerically the current argument you're on
	 *
	 *
	 */

	int num_args = 0;		
	int buff_offset = 0;	
	int arg_buff_count = 0;	
	int buff_size = 0;		
	int is_first = 0;		// flag to know if it is the first char in argument
	int cur_arg = 0;

	if (program == NULL) {
		*retval = EFAULT;
		return -1;
	}

	if (args == NULL) {
		*retval = EFAULT;
		return -1;
	}

/*	err = copyin((const_userptr_t)*args,(void *)pointer_char, 1);
	if (err) {
		*retval = EFAULT;
		return -1;
	}
*/
	
	char kernel_progname[128];
	err = copyinstr((const_userptr_t)program, kernel_progname, 128, 0);
	if (err) {
		*retval = EFAULT;
		return -1;
	}

	//LETS JENK THE SHIT OUT OF THIS



	/* copys the argument list into a safe variable (on the kernel heap... or 
	 * stack.... i still dont know how memory works.
	 */
/*	
	copyin((const_userptr_t)args , &safe_args, sizeof(args));
	kprintf("what is in safe_args = %p\n",(void *)safe_args);
	kprintf("(char *)safe_args = %s\n",(char *)safe_args);
*/	
	
	/* counts the number of arguments */	
/*	while (*(char **)safe_args != NULL) {
		
		
		buff_offset++;
		args++;
		safe_args++;
		num_args++;
		copyin((const_userptr_t)args , &safe_args, 4);
		kprintf("what is in safe_args = %p\n",(void *)safe_args);
		kprintf("(char *)safe_args = %s\n",(char *)safe_args);
	}
*/
	
	while(*args != NULL){

		buff_offset++;
		args++;
		num_args++;
	}

//	kprintf("this should be 3 = %d\n",num_args);


	args -= buff_offset;

//	*safe_args -= buff_offset;
//		kprintf("args is = %p\n", (void *)args);
//		kprintf("buff_offset = %d\n", buff_offset);
//		kprintf("    what is in safe_args = %p\n",(void *)safe_args);
//		kprintf("    (char *)safe_args = %s\n",(char *)safe_args);


	buff_offset += 1;  //for the null pointer buffer between pointers and chars
	buff_offset = (buff_offset * 4);
	pointer_char += buff_offset;


	// kprintf("THIS IS THE KERNEL BUFFER\n");
	
	/*copys in the arguments from user space to the kernel buffer character
	 * by character.
	 */


	/* loops through each argument one by one */
	while (cur_arg != num_args) { 

		int arg_len = 0;	
		is_first = 1;
//	kprintf("cur_arg = %d      |      num_args = %d\n", cur_arg,num_args);

//		kprintf("what it this = %s\n", (char *)safe_args);
//		kprintf("where is this = %p\n", (void *)safe_args);
		/* grabs each character from the argument one by one */

		//		panic("1\n");
//		while (*(char *)safe_args != 0) {
		while(**args != 0){
	//		kprintf("buffer size = %d\n", buff_size);
			arg_buff_count++;
			buff_size++;

			/* if the character is the first character in the argument we 
			 * assign the pointer to point to it
			 */
			if(is_first){
			
				is_first = 0;
				pointer_void =(void *) pointer_char;
				
//				kprintf("k_pointer = %p\n", pointer_void);
//				kprintf("k_pointer -> %s\n", (char *)pointer_void);
				
				pointer_void+=4;
				
//			kprintf("+4 because pointer buffer size = %d\n", buff_size);
				buff_size+=4;
			}
			/*then we copy the character in from user space */

//			kprintf("why is this breaking = %c\n", *(char *)safe_args);
//			copyin((const_userptr_t)*safe_args, &pointer_char,1);
					
			copyin((const_userptr_t)*args,(void *)pointer_char, 1);
			if (err) {
				*retval = EFAULT;
				return -1;
			}

//			kprintf("k_arguments = %s        address = %p \n",pointer_char, (void *)pointer_char);
			
			*args += 1;
//			*safe_args+=1;
			arg_len++;
			pointer_char+=1;
			// cur_arg += 1;
	
		}
		
		/*THIS IS WHERE WE SET THE PADDING */




	//	kprintf("4050%4: %d\n", 4050%4);
		int padding = 4-(arg_len % 4);
		if (padding) {
		
			if (arg_len < 4) {
				padding = 4 - arg_len;
			}
			
			int i;
//			kprintf("padding = %d\n", padding);	
			for (i = 0; i < padding; i++) {
				arg_buff_count++;
				buff_size++;
				*pointer_char = '\0';
				// kprintf("k_arguments = %s       address = %p \n",pointer_char, (void *)pointer_char);

				//			kprintf("buffer size = %d\n", buff_size);
				pointer_char++;
			}
		}
		else {
			int i;
//			kprintf("padding = %d\n", padding);	
			for (i = 0; i < 4; i++) {
				arg_buff_count++;
				buff_size++;
//				*pointer_char = '\0';
	
					// kprintf("k_arguments = %s       address = %p \n",pointer_char, (void *)pointer_char);
//			kprintf("buffer size = %d\n", buff_size);
				pointer_char++;
		
			}
		}

		/*if the arguments length isnt a multiple of 4 */ 

/*   
		if((arg_len%4)){
			// if the argument is less than 4
			if(arg_len < 4){
		
				int i;
				// add the correct padding 
				for(i = 0; i < 4-arg_len; i++){

					arg_buff_count++;
					buff_size++;
					*pointer_char = '\0';
					
//					kprintf("k_arguments = %s       address = %p \n",pointer_char, (void *)pointer_char);
					
					pointer_char++;
				}
			} // if the argument is greater than 4
			else{
				
				int i;
				// calculate the correct padding and add it
				for(i = 0; i< arg_len%4; i++){
				
					arg_buff_count++;
					buff_size++;
					*pointer_char = '\0';
					
//					kprintf("k_arguments = %s       address = %p \n",pointer_char, (void *)pointer_char);
				
					pointer_char+=1;
				}
			}
		} // if the arguments length is a multiple of 4
		else{
			int i;
			// add 4 padding to it 
			for (i = 0; i<4; i++) {
				
				arg_buff_count++;
				buff_size++;
				*pointer_char = '\0';
				
//				kprintf("k_arguments = %s       address = %p \n",pointer_char, (void *)pointer_char);
				
				pointer_char+=1;
			}
		}
*/
//kprintf("buffer size %d\n",buff_size);

		args += 1;
//		safe_args += 1;
		cur_arg += 1;
	
	}
	buff_size+=4;
	
//			kprintf(" null pointer buffer size = %d\n", buff_size);
	


	/*sets the void pointer on the kernel buffer*/
	//pointer_void = NULL;
	
//	kprintf("k_pointer = %p\n", pointer_void);
//	kprintf("k_pointer -> %s\n", (char *)pointer_void);


//END OF MY CODE SECTION





	/* BEGIN RUNPROGRAM  */

	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	/* Open the file. */
	// char kernel_progname[128];
	copyinstr((const_userptr_t)program, kernel_progname, 128, 0);
	
	result = vfs_open(kernel_progname, O_RDONLY, 0, &v);
	if (result) {
//	kprintf("error = %d\n", result);
		return result;
	}

	/* We should be a new process. */
//	KASSERT(proc_getas() == NULL); //we commented this out......

	/* Create a new address space. */
	as = as_create();
	if (as == NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Switch to it and activate it. */
	proc_setas(as);
	as_activate();

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}	
	




	/* START OF MY CODE */

	
	
	
	



	// kprintf("\n\n\n\n\nTHIS IS THE USER STACK\n");

//	kprintf("size of buffer = %d\n", buff_size);
//	kprintf("argument buffer len = %d\n",arg_buff_count);




	/*
	 * arg_pointer - pointer to user stack that is used for copying out the 
	 *               arguments
	 *
	 * top_of_stack - pointer to user stack that is used for copying out the
	 *                pointers to the arguments
	 *
	 * entry_stack - pointer to the bottom of the user stack that we pass into
	 *               enter_new_process as the user pointer
	 *
	 * static_top_of_stack - this pointer will always point to the top of 
	 *                       the user stack
	 *
	 *
	 * new_arg - a flag if false->we arent on the first character of an 
	 *           argument. if true-> we are.
	 *
	 */

	vaddr_t static_top_of_stack = stackptr;
	vaddr_t arg_pointer = static_top_of_stack - arg_buff_count;
	vaddr_t top_of_stack = static_top_of_stack - buff_size;
	vaddr_t entry_stack = static_top_of_stack - buff_size;

	/*
	 * new_arg - another flag to show if we've hit the first character in a
	 *           new argument....... why do i keep making new variables....
	 *
	 */
	
	int new_arg = true;
	int i;
	
//	stackptr-= buff_size;
//	stackptr -= 4; //skips the null ptr
	cur_arg = 0;
	k_buff += buff_offset;
	

	/*goes through the kernel buffer and copys out to the user stack
	 *character by characteer
	 */
	for (i = buff_offset; i < buff_size; i++) {
			
		copyout((const void *)k_buff, (userptr_t)arg_pointer, 1);
		
//		kprintf("agruments = \'%s\'  address = %p \n", (char *)arg_pointer, (void *)arg_pointer);

		/*when it passes this if statement that means that weve hit the first
		 *character in the argument, so we set a pointer to point to that 
		 *location
		 */
		if (new_arg && *(char *)arg_pointer != 0) {
			
			top_buff = (void *)arg_pointer;
			
			//top_buff = (void *)0x7fffffe8 ;
			
			//kprintf("brijesh : %s\n", (char *)top_buff);
			
			copyout(&top_buff, (userptr_t)top_of_stack, 4);
			
			//memcpy((void *)top_of_stack, &top_buff, 4);

			// kprintf("pointer = %p\n", (void *)top_of_stack);

			





//			kprintf("pointer -> %s\n",*(char **)top_of_stack);
			
			top_of_stack += 4;
			top_buff += 4;
			new_arg = false;
		
		}

		 // kprintf("arg_pointer: %c              address = %p\n", *(char *)arg_pointer, (void*)arg_pointer);
		
		/*this sets the flag to show weve ended an argument and should now 
		 *start looking for the next argument starting point
		 */
		if (*(char *)arg_pointer == 0) {
		
			new_arg = true;
			
		}
		
		arg_pointer +=1;
		k_buff += 1;
	}
	
	/*copys in the void pointer*/ 
	//copyout((const void *)top_buff, (userptr_t)top_of_stack, 4);

	//kprintf("pointer = %p\n",(void *)top_of_stack);
	//kprintf("pointer ->\' %s\'\n",(char *)top_of_stack);
	top_of_stack += 4;
	

	// kprintf("stackptr: %p\n", (void *)stackptr);
	
	// kprintf("num_args %d\n", (num_args));
	// kprintf("buff_offset %d\n", buff_offset);
	// kprintf("arg_buff_count %d\n", arg_buff_count);
	// kprintf("buff_size %d\n", buff_size);

	/* Warp to user mode. */

	/*just a sanity check to see if the values are in the correct spot*/

	/*kprintf("\n\n\n\nsanity check\n");

	vaddr_t sanity_arg = static_top_of_stack - buff_size;
	vaddr_t sanity = static_top_of_stack - arg_buff_count;
	new_arg = true;
	int hullo;
	
	for (hullo = 0; hullo < arg_buff_count; hullo++) {
	





			
		kprintf("agruments = \'%s\'  address = %p \n", 
			 (char *)sanity, (void *)sanity);


		if (new_arg && *(char *)sanity != 0) {

			kprintf("pointer = %p\n", (void *)sanity_arg);
			kprintf("pointer -> %s\n",(char *)sanity_arg);

			sanity_arg += 4;
			new_arg = false;
		
		}

		// kprintf("arg_pointer: %d =? 0\n", *(char *)arg_pointer);
		if (*(char *)sanity == 0) {
		
			new_arg = true;
			
		}
		
		sanity +=1;
	
	}


	kprintf("DAVE: %s\n", (char *)0x7fffffd8);
*/


	// kprintf("\n\n\n\n\nnum_args = %d\n", num_args);
	// kprintf("entry_stack = %p\n", (void *)entry_stack);

	vaddr_t bottom_of_stack = static_top_of_stack- buff_size;
	
	// kprintf("bottom_of_stack = %p\n", (void *)bottom_of_stack);
	// kprintf("entrypoint = %p\n\n", (void *)entrypoint);
	
	
	enter_new_process(num_args,			/* argc  */ 
			(userptr_t)entry_stack,		/* userspace addr of argv */
			NULL,						/* userspace addr of environment */
			bottom_of_stack,entrypoint);


	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	*retval =  EINVAL;
	return -1;
}

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

//TODO:
//- if the argument is divisible by 4 then you need to add 4 null points of 
//padding.
//
//-keep a count of the total lenght of the arguments.
//
//- reassigning pointer position is going to be the arg_buff_count +4+(4*count)
// count being the current pointer youre on

	(void)retval;
	
	// kprintf("CALL TO EXEC CONNECTED\n");

	bzero(k_buff, ARG_MAX);

	//kprintf("buff loc: %p\n", k_buff);


//	char single_char = (char)k_buff;
	void *pointer_void = k_buff;
	void *top_buff = k_buff;
	char *pointer_char = (char *)k_buff;

	//char *temp_ptr = pointer_char + arg_counter;
	int num_args = 0;		// number of arguments
	int buff_offset = 0;	// total number of bytes of all pointers (including NULL)
	int arg_buff_count = 0;	// amount of bytes arguments + buffers take
	int buff_size = 0;		// bytes of pointers + args

	while (*args != NULL) {
		//kprintf("args : %p\n",args);
		buff_offset++;
		args++;
		num_args++;
	}

	args -= buff_offset;
	buff_offset += 1;		// for the null pointer buffer between pointers and chars
	buff_offset = (buff_offset * 4);

	pointer_char += buff_offset;
	int cur_arg = 0;

	int is_first = 0;		// flag to know if it is the first char in argument

	//copys the arguments character by character and adds a buffer also 
	//sets the pointers
	kprintf("THIS IS THE KERNEL BUFFER\n");
	
	while (cur_arg != num_args) {
		int arg_len = 0;	
		is_first = 1;
		while (**args != 0) {
			arg_buff_count++;
			buff_size++;
			//assignes the pointer
			if(is_first){
				is_first = 0;
				pointer_void =(void *) pointer_char;
				kprintf("k_pointer = %p\n", pointer_void);
				kprintf("k_pointer -> %s\n", (char *)pointer_void);
				pointer_void+=4;
				buff_size+=4;
			}
			copyin((const_userptr_t)*args,pointer_char,1);
			kprintf("k_arguments = %s        address = %p \n",pointer_char, (void *)pointer_char);
			//kprintf("please dont be 0x7... = %p\n", (void *)pointer_char);
			*args+=1;
			arg_len++;
			pointer_char+=1;
		}
		if((arg_len%4)){
			if(arg_len < 4){
				int i;
				for(i = 0; i < 4-arg_len; i++){
					arg_buff_count++;
					buff_size++;
					*pointer_char = '\0';
					kprintf("k_arguments = %s       address = %p \n",pointer_char, (void *)pointer_char);
					pointer_char++;
				}
			}
			else{
				int i;
				for(i = 0; i< arg_len%4; i++){
					arg_buff_count++;
					buff_size++;
					*pointer_char = '\0';
					kprintf("k_arguments = %s       address = %p \n",pointer_char, (void *)pointer_char);
					//kprintf("please be 0x7...  = %p\n",(void *)pointer_char);
				
					pointer_char+=1;
				}
			}
		}
		else{
			int i;
			for (i = 0; i<4; i++) {
				arg_buff_count++;
				buff_size++;
				*pointer_char = '\0';
				kprintf("k_arguments = %s       address = %p \n",pointer_char, (void *)pointer_char);
				pointer_char+=1;
			}
		}
		args += 1;
		cur_arg+=1;
	}
	buff_size+=4;
	//kprintf("the length of the arg buffer 36 = %d\n",buff_size);
	pointer_void = NULL;
	kprintf("k_pointer = %p\n", pointer_void);
	kprintf("k_pointer -> %s\n", (char *)pointer_void);

	/* BEGIN RUNPROGRAM  */

	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	/* Open the file. */
	char kernel_progname[128];
	copyinstr((const_userptr_t)program, kernel_progname, 128, 0);

	// kprintf("progname: %s\n", kernel_progname);
	result = vfs_open(kernel_progname, O_RDONLY, 0, &v);
	if (result) {
	kprintf("error = %d\n", result);
		return result;
	}

	/* We should be a new process. */
	// KASSERT(proc_getas() == NULL);

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
	//kprintf("top of user stack 0x7fffffff = %p\n", (void *)stackptr-1);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}	
	
	// I dont think ive reset the pointers for userland.......... kill me	
	// copy the arguments into the user stack. NOT GEOFF'S CODE

	vaddr_t static_top_of_stack = stackptr;

//	stackptr-= arg_buff_count;
	stackptr-= buff_size;
	// kprintf("bottom of arg buff 0x77ffffffdc = %p\n", (void *)stackptr);
	stackptr -= 4; //skips the null ptr
	cur_arg = 0;

	/*
	//this is not right....... fine ish....
	while(cur_arg != num_args){
		cur_arg++;
		stackptr += 4;

		top_buff = (void *)stackptr;
		kprintf("stack pointer = %p\n", (void *)stackptr);
	//	kprintf("k_buffer pointer = %p\n", top_buff);
		top_buff+=4;
	}
*/

//	stackptr-=num_args*4;	
//	top_of_stack = stackptr;
	kprintf("THIS IS THE USER STACK\n");

	/*
	//int pointers = 0;
	is_first = 1;
	int found_null = 0;
	*/

	// kprintf("hullo %p\n", (void *)stackptr);	

	vaddr_t arg_pointer = static_top_of_stack - arg_buff_count;
	vaddr_t top_of_stack = static_top_of_stack - buff_size;
	vaddr_t entry_stack = static_top_of_stack - buff_size;

	k_buff += buff_offset;
	
	int new_arg = true;

	int i = buff_offset;
	for (; i < buff_size; i++) {
			
		copyout((const void *)k_buff, (userptr_t)arg_pointer, 1);
		 kprintf("agruments = \'%s\'  address = %p \n", 
			   (char *)arg_pointer, (void *)arg_pointer);


		if (new_arg && *(char *)arg_pointer != 0) {
			
			top_buff = (void *)arg_pointer;

			copyout((const void *)top_buff, (userptr_t)top_of_stack, 4);
			kprintf("pointer = %p\n", (void *)top_of_stack);
			kprintf("pointer -> %s\n",(char *)top_of_stack);
			top_of_stack += 4;
			top_buff += 4;

			new_arg = false;
		}

		// kprintf("arg_pointer: %d =? 0\n", *(char *)arg_pointer);
		if (*(char *)arg_pointer == 0) {
			new_arg = true;
			// kprintf("found\n");
		}
		
		arg_pointer +=1;
		k_buff += 1;
	}
	//(void *)top_of_stack = NULL;
	copyout((const void *)top_buff, (userptr_t)top_of_stack, 4);
	kprintf("pointer = %p\n",(void *)top_of_stack);
	kprintf("pointer ->\' %s\'\n",(char *)top_of_stack);
/* 
	for (i = (num_args+1)*4; i < buff_size; i++) {
		if (i >= 0 && i < ((num_args+1)*4)) {
				
			copyout((const void *)k_buff,(userptr_t)top_of_stack,4);
			kprintf("pointer -> %p\n",(void *)top_of_stack);
			top_of_stack+=4;
			k_buff+=4;
			i+=3;	
		}
		else {

		if(is_first){
			is_first = 0;
			stack_buff = (void *)stackptr;
			copyout((const void *)stack_buff,(userptr_t)top_of_stack,4);
			stack_buff++;
			top_of_stack+=4;


		}
		if((char *)k_buff == '\0' || found_null){
			found_null = 1;
			if((char *)k_buff != '\0'){
				found_null = 0;
				is_first = 1;
			}
	
		}
		
			copyout((const void *)k_buff,(userptr_t)top_of_stack,1);
			kprintf("agruments = %s   address = %p \n", (char *)top_of_stack,(void *)top_of_stack);
			top_of_stack+=1;
			k_buff+=1;
		}
	}
*/

	// kprintf("stackptr: %p\n", (void *)stackptr);
	
	// kprintf("num_args %d\n", (num_args));
	// kprintf("buff_offset %d\n", buff_offset);
	// kprintf("arg_buff_count %d\n", arg_buff_count);
	// kprintf("buff_size %d\n", buff_size);

	/* Warp to user mode. */




	kprintf("\n\nnum_args = %d\n", num_args);
	kprintf("entry_stack = %p\n", (void *)entry_stack);
	vaddr_t bottom_of_stack = static_top_of_stack- buff_size;
	kprintf("bottom_of_stack = %p\n\n", (void *)bottom_of_stack);
	enter_new_process(num_args,			/* argc  */ 
			(userptr_t)entry_stack,		/* userspace addr of argv */
			NULL,						/* userspace addr of environment */
			bottom_of_stack, entrypoint);


	/* enter_new_process does not return. */
	// panic("enter_new_process returned\n");
	*retval =  EINVAL;
	return -1;
}

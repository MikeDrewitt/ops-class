/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <current.h>
#include <syscall.h>
#include <copyinout.h>
#include <addrspace.h>
#include <proc.h>

#include <kern/errno.h>
#include <kern/syscall.h>
#include <mips/trapframe.h>

/*
 * System call dispatcher.
 *
 * A pointer to the trapframe created during exception entry (in
 * exception-*.S) is passed in.
 *
 * The calling conventions for syscalls are as follows: Like ordinary
 * function calls, the first 4 32-bit arguments are passed in the 4
 * argument registers a0-a3. 64-bit arguments are passed in *aligned*
 * pairs of registers, that is, either a0/a1 or a2/a3. This means that
 * if the first argument is 32-bit and the second is 64-bit, a1 is
 * unused.
 *
 * This much is the same as the calling convpid_t sys_fork();tions for ordinary
 * function calls. In addition, the system call number is passed in
 * the v0 register.
 *
 * On successful return, the return value is passed back in the v0
 * register, or v0 and v1 if 64-bit. This is also like an ordinary
 * function call, and additionally the a3 register is also set to 0 to
 * indicate success.
 *
 * On an error return, the error code is passed back in the v0
 * register, and the a3 register is set to 1 to indicate failure.
 * (Userlevel code takes care of storing the error code in errno and
 * returning the value -1 from the actual userlevel syscall function.
 * See src/user/lib/libc/arch/mips/syscalls-mips.S and related files.)
 *
 * Upon syscall return the program counter stored in the trapframe
 * must be incremented by one instruction; otherwise the exception
 * return code will restart the "syscall" instruction and the system
 * call will repeat forever.
 *
 * If you run out of registers (which happens quickly with 64-bit
 * values) further arguments must be fetched from the user-level
 * stack, starting at sp+16 to skip over the slots for the
 * registerized values, with copyin().
 */
void
syscall(struct trapframe *tf)
{
	int callno;
	int32_t retval;
	int err;
	
	int64_t retval_long;
	int64_t  full_pos;
	int seek = 0;
	int bad_sys = 0;
	
	KASSERT(curthread != NULL);
	KASSERT(curthread->t_curspl == 0);
	KASSERT(curthread->t_iplhigh_count == 0);
	
	curproc->p_tf = tf;

	callno = tf->tf_v0;

	/*
	 * Initialize retval to 0. Many of the system calls don't
	 * really return a value, just 0 for success and -1 on
	 * error. Since retval is the value returned on success,
	 * initialize it to 0 by default; thus it's not necessary to
	 * deal with it except for calls that return other values,
	 * like write.
	 */

	retval = 0;

	// kprintf("current trap status: %x\n", tf->tf_status);
	// kprintf("call to syscall %d\n", callno);

	switch (callno) {
		case SYS_fork:
			err = sys_fork(&retval);
		break;

		case SYS_waitpid:
			err = sys_waitpid(&retval, (pid_t)tf->tf_a0, (int *)tf->tf_a1, (int)tf->tf_a2);
		break;

		case SYS_getpid:
			err = sys_getpid(&retval);
		break;

		case SYS_reboot:
			err = sys_reboot(tf->tf_a0);
		break;

		case SYS_open:
			err = sys_open(&retval, (const char *)tf->tf_a0, (int)tf->tf_a1);
		break;

		case SYS_close:
			err = sys_close(&retval, (int)tf->tf_a0);
		break;

		case SYS_read:
			err = sys_read(&retval, (int)tf->tf_a0, (void *)tf->tf_a1, (size_t)tf->tf_a2);
		break;

		case SYS_write:
			err = sys_write((int)tf->tf_a0, (const void *)tf->tf_a1, (size_t)tf->tf_a2, &retval);
		break;

		case SYS___time:
			err = sys___time((userptr_t)tf->tf_a0,
				 (userptr_t)tf->tf_a1);
		break;

		case SYS_execv:
			err = sys_execv(&retval, (const char *)tf->tf_a0, (char **)tf->tf_a1);
		break;

		case SYS__exit:
			/* Not sure what to assign err to here, so I just zero it out  */
			sys__exit(&retval, (int) tf->tf_a0);
			// kprintf("RETURN FROM EXIT: current trap cause: %x\n", tf->tf_cause);
			err = 0;
		break;
			
	    case SYS_lseek:
			full_pos = tf->tf_a2;
			full_pos = full_pos << 32;
			full_pos += tf->tf_a3;

			int whence;
			copyin((const_userptr_t)tf->tf_sp+16, &whence, sizeof(int));

			err = sys_lseek(&retval_long ,(int)tf->tf_a0, (off_t)full_pos,(int) whence);	
			seek = 1;
		break;
	
		case SYS_dup2:
			err = sys_dup2(&retval, (int)tf->tf_a0, (int)tf->tf_a1);
		break;	
		/* Add stuff here */

	    default:
		kprintf("Unknown syscall %d\n", callno);
		err = -1;
		bad_sys = 1;
		retval = ENOSYS;
		break;
	}


	if (err) {
		/*
		 * Return the error code. This gets converted at
		 * userlevel to a return value of -1 and the error
		 * code in errno.
		 */
	
		if (seek){ retval = retval_long;}
		if(bad_sys){
			tf->tf_v0 = retval;
			tf->tf_a3 = -1;

		}
		else{

			tf->tf_v0 = retval;
			tf->tf_a3 = -1;     /* signal an error */
		}
	}
	else {
		/* Success. */
		if (seek) {
			// kprintf("our_answer: %d\n", retval);
			// tf->tf_v1 = retval & 0xFFFFFFFF; // hopefully this grabs bottom 32
			// tf->tf_v0 = (retval & 0xFFFFFFFF00000000) >> 32; // and this grabs top 32
			tf->tf_v1 = retval_long;
			tf->tf_v0 = retval_long >> 32;
			seek = 0;
		}
		else {
			tf->tf_v0 = retval;
		} 		
		tf->tf_a3 = 0;      /* signal no error */
	}

	/*
	 * Now, advance the program counter, to avoid restarting
	 * the syscall over and over again.
	 */

	tf->tf_epc += 4;
	(void)seek;

	/* Make sure the syscall code didn't forget to lower spl */
	KASSERT(curthread->t_curspl == 0);
	/* ...or leak any spinlocks */
	KASSERT(curthread->t_iplhigh_count == 0);

	// kprintf("END SYSCALL: current trap cause: %x\n", tf->tf_cause);
	
	// kprintf("end syscall %d\n", callno);
	// kprintf("v0: %lld\n", (long long int)tf->tf_v0);
	// kprintf("v1: %lld\n", (long long int)tf->tf_v1);
	// kprintf("finish syscall\n");
}

/*
 * Enter user mode for a newly forked process.
 *
 * This function is provided as a reminder. You need to write
 * both it and the code that calls it.
 *
 * Thus, you can trash it and do things another way if you prefer.
 */

void
enter_forked_process(void *data1, unsigned long data2)
{		
	struct trapframe tf;

	memcpy(&tf, curproc->p_tf, sizeof(*curproc->p_tf));

	tf.tf_v0 = 0;
	tf.tf_a3 = 0;
	tf.tf_epc += 4;

	kprintf("trap cause: %x\n", tf.tf_cause);

	as_activate();
	mips_usermode(&tf);

	(void)data1;
	(void)data2;

	return;
}

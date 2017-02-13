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

/*
 * Synchronization primitives.
 * The specifications of the functions are in synch.h.
 */

#include <types.h>
#include <lib.h>
#include <spinlock.h>
#include <wchan.h>
#include <thread.h>
#include <current.h>
#include <synch.h>

////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *name, unsigned initial_count)
{
	struct semaphore *sem;

	sem = kmalloc(sizeof(*sem));
	if (sem == NULL) {
		return NULL;
	}

	sem->sem_name = kstrdup(name);
	if (sem->sem_name == NULL) {
		kfree(sem);
		return NULL;
	}

	sem->sem_wchan = wchan_create(sem->sem_name);
	if (sem->sem_wchan == NULL) {
		kfree(sem->sem_name);
		kfree(sem);
		return NULL;
	}

	spinlock_init(&sem->sem_lock);
	sem->sem_count = initial_count;

	return sem;
}

void
sem_destroy(struct semaphore *sem)
{
	KASSERT(sem != NULL);

	/* wchan_cleanup will assert if anyone's waiting on it */
	spinlock_cleanup(&sem->sem_lock);
	wchan_destroy(sem->sem_wchan);
	kfree(sem->sem_name);
	kfree(sem);
}

void
P(struct semaphore *sem)
{
	KASSERT(sem != NULL);

	/*
	 * May not block in an interrupt handler.
	 *
	 * For robustness, always check, even if we can actually
	 * complete the P without blocking.
	 */
	KASSERT(curthread->t_in_interrupt == false);

	/* Use the semaphore spinlock to protect the wchan as well. */
	spinlock_acquire(&sem->sem_lock);
	while (sem->sem_count == 0) {
		wchan_sleep(sem->sem_wchan, &sem->sem_lock);
	}
	KASSERT(sem->sem_count > 0);
	sem->sem_count--;
	spinlock_release(&sem->sem_lock);
}

void
V(struct semaphore *sem)
{
	KASSERT(sem != NULL);

	spinlock_acquire(&sem->sem_lock);

	sem->sem_count++;
	KASSERT(sem->sem_count > 0);
	wchan_wakeone(sem->sem_wchan, &sem->sem_lock);

	spinlock_release(&sem->sem_lock);
}

///////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
	struct lock *lock;
	
	lock = kmalloc(sizeof(*lock));
	if (lock == NULL) {
		return NULL;
	}

	lock->lk_name = kstrdup(name);
	if (lock->lk_name == NULL) {
		kfree(lock);
		return NULL;
	}
//added stuff below
	lock->lock_wchan = wchan_create(lock->lk_name);
	if (lock->lock_wchan == NULL) {
		kfree(lock->lk_name);
		kfree(lock);
		return NULL;
	}
	
	lock->lock_thread = NULL;
	spinlock_init(&lock->lock_lock);
// added stuff above

	HANGMAN_LOCKABLEINIT(&lock->lk_hangman, lock->lk_name);

	// add stuff here as needed

	return lock;
}

void
lock_destroy(struct lock *lock)
{
	KASSERT(lock != NULL);

// added stuff below
	KASSERT(!lock_do_i_hold(lock));
	spinlock_cleanup(&lock->lock_lock);
	wchan_destroy(lock->lock_wchan);

//added stuff above
	kfree(lock->lk_name);
	kfree(lock);
}
//this is basically P for semaphores

void
lock_acquire(struct lock *lock)
{
	KASSERT(lock != NULL);
	//KASSERT(curthread->t_in_interrupt == false);
	
	spinlock_acquire(&lock->lock_lock);
	
	while (lock->lock_thread != NULL) {
		wchan_sleep(lock->lock_wchan, &lock->lock_lock);
	}
	
	lock->lock_thread = curthread;

	spinlock_release(&lock->lock_lock);
	/* Call this (atomically) before waiting for a lock */
	//HANGMAN_WAIT(&curthread->t_hangman, &lock->lk_hangman);

	// Write this


	/* Call this (atomically) once the lock is acquired */
	//HANGMAN_ACQUIRE(&curthread->t_hangman, &lock->lk_hangman);
}

//basically V for semaphores
void
lock_release(struct lock *lock)
{
	KASSERT(lock != NULL);
	KASSERT(lock->lock_thread != NULL);
	KASSERT(lock_do_i_hold(lock));

	spinlock_acquire(&lock->lock_lock);
		
	lock->lock_thread = NULL;
	wchan_wakeone(lock->lock_wchan, &lock->lock_lock);
	/* Call this (atomically) when the lock is released */
	//HANGMAN_RELEASE(&curthread->t_hangman, &lock->lk_hangman);

	// Write this

	spinlock_release(&lock->lock_lock);
}


/*
 * Returns true if the lock is held by the thread,
 * false if it is held by someone else, or not held at all.
 */
bool
lock_do_i_hold(struct lock *lock)
{
	KASSERT(lock != NULL);
	return curthread == lock->lock_thread;
}

/*
 * Implementation on CV
 */
struct cv *
cv_create(const char *name)
{
	struct cv *cv;

	cv = kmalloc(sizeof(*cv));
	if (cv == NULL) {
		return NULL;
	}

	cv->cv_name = kstrdup(name);
	if (cv->cv_name==NULL) {
		kfree(cv);
		return NULL;
	}

// BEGIN 
	cv->cv_wchan = wchan_create(cv->cv_name);
	if (cv->cv_wchan == NULL) {
		kfree(cv->cv_name);
		kfree(cv);
		return NULL;
	}
// END
	return cv;
}

void
cv_destroy(struct cv *cv)
{
	KASSERT(cv != NULL);

// BEGIN
	wchan_destroy(cv->cv_wchan);
// END

	 kfree(cv->cv_name);
	kfree(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
// BEGIN
	KASSERT(cv != NULL);
	KASSERT(lock != NULL);
	KASSERT(lock->lock_thread != NULL);

	spinlock_acquire(&lock->lock_lock);	
	if (lock->lock_thread == curthread) {

		lock->lock_thread = NULL;
		wchan_wakeone(lock->lock_wchan, &lock->lock_lock);
	
		wchan_sleep(cv->cv_wchan, &lock->lock_lock);
	
		while (lock->lock_thread != NULL) {
			wchan_sleep(lock->lock_wchan, &lock->lock_lock);
		}
		
		lock->lock_thread = curthread;

	}
	
	spinlock_release(&lock->lock_lock);
// END
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
// BEGIN
	KASSERT(cv != NULL);
	KASSERT(lock != NULL);
	KASSERT(lock->lock_thread != NULL);

	spinlock_acquire(&lock->lock_lock);
	wchan_wakeone(cv->cv_wchan, &lock->lock_lock);
	spinlock_release(&lock->lock_lock);
// END
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
// BEGIN
	KASSERT(cv != NULL);
	KASSERT(lock != NULL);
	KASSERT(lock->lock_thread != NULL);

	spinlock_acquire(&lock->lock_lock);
	wchan_wakeall(cv->cv_wchan, &lock->lock_lock);
	spinlock_release(&lock->lock_lock);
// END
}

struct rwlock *
rwlock_create(const char *name)
{
	struct rwlock *rwlock;

	rwlock = kmalloc(sizeof(*rwlock));

	if (rwlock == NULL) {
		return NULL;
	}

	rwlock->rwlock_name = kstrdup(name);
	if (rwlock->rwlock_name == NULL) {
		kfree(rwlock);
		return NULL;
	}

	spinlock_init(&rwlock->rwlock_lock);

	rwlock->read_count = 0;
	rwlock->write_count = 0;

	rwlock->rwlock_readwchan = wchan_create(rwlock->rwlock_name);
	rwlock->rwlock_writewchan = wchan_create(rwlock->rwlock_name);

	return rwlock;
}

void
rwlock_destroy(struct rwlock *rwlock)
{
	KASSERT(rwlock != NULL);

	spinlock_cleanup(&rwlock->rwlock_lock);

	wchan_destroy(rwlock->rwlock_readwchan);
	wchan_destroy(rwlock->rwlock_writewchan);

	kfree(rwlock->rwlock_name);
	kfree(rwlock);
}

void 
rwlock_acquire_read(struct rwlock *rwlock)
{
	spinlock_acquire(&rwlock->rwlock_lock);

	// if writing currently, wait.
	// else aquire read.
	while (rwlock->write_count != 0 || 
		!wchan_isempty(rwlock->rwlock_writewchan, &rwlock->rwlock_lock)) {
		wchan_sleep(rwlock->rwlock_readwchan, &rwlock->rwlock_lock);
	}
	
	rwlock->read_count++;

	spinlock_release(&rwlock->rwlock_lock);
}

void
rwlock_release_read(struct rwlock *rwlock)
{
	spinlock_acquire(&rwlock->rwlock_lock);

	rwlock->read_count--;

	if (rwlock->read_count == 0) {
		wchan_wakeall(rwlock->rwlock_writewchan, &rwlock->rwlock_lock);
		wchan_wakeall(rwlock->rwlock_readwchan, &rwlock->rwlock_lock);
	}	

	spinlock_release(&rwlock->rwlock_lock);
}
	
void 
rwlock_acquire_write(struct rwlock *rwlock)
{
	spinlock_acquire(&rwlock->rwlock_lock);

	// if reading or writing wait
	// else aqurire write. 
	while (rwlock->write_count != 0 || rwlock->read_count != 0) {
		wchan_sleep(rwlock->rwlock_writewchan, &rwlock->rwlock_lock);
	}	

	rwlock->write_count++;
	spinlock_release(&rwlock->rwlock_lock);
}

void
rwlock_release_write(struct rwlock *rwlock)
{	
	spinlock_acquire(&rwlock->rwlock_lock);

	wchan_wakeall(rwlock->rwlock_readwchan, &rwlock->rwlock_lock);
	wchan_wakeall(rwlock->rwlock_writewchan, &rwlock->rwlock_lock);
	
	rwlock->write_count--;
	spinlock_release(&rwlock->rwlock_lock);
}

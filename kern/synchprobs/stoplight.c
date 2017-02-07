/*
 * Copyright (c) 2001, 2002, 2009
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
 * Driver code is in kern/tests/synchprobs.c We will replace that file. This
 * file is yours to modify as you see fit.
 *
 * You should implement your solution to the stoplight problem below. The
 * quadrant and direction mappings for reference: (although the problem is, of
 * course, stable under rotation)
 *
 *   |0 |
 * -     --
 *    01  1
 * 3  32
 * --    --
 *   | 2|
 *
 * As way to think about it, assuming cars drive on the right: a car entering
 * the intersection from direction X will enter intersection quadrant X first.
 * The semantics of the problem are that once a car enters any quadrant it has
 * to be somewhere in the intersection until it call leaveIntersection(),
 * which it should call while in the final quadrant.
 *
 * As an example, let's say a car approaches the intersection and needs to
 * pass through quadrants 0, 3 and 2. Once you call inQuadrant(0), the car is
 * considered in quadrant 0 until you call inQuadrant(3). After you call
 * inQuadrant(2), the car is considered in quadrant 2 until you call
 * leaveIntersection().
 *
 * You will probably want to write some helper functions to assist with the
 * mappings. Modular arithmetic can help, e.g. a car passing straight through
 * the intersection entering from direction X will leave to direction (X + 2)
 * % 4 and pass through quadrants X and (X + 3) % 4.  Boo-yah.
 *
 * Your solutions below should call the inQuadrant() and leaveIntersection()
 * functions in synchprobs.c to record their progress.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

/*
 * Called by the driver during initialization.
 */


struct intersection *inter;

void
stoplight_init() {
	
	inter = kmalloc(sizeof(struct intersection));

	inter->top_left = lock_create("top_left");
	inter->top_right = lock_create("top_right");
	inter->bottom_left = lock_create("bottom_left");
	inter->bottom_right = lock_create("bottom_right");

	inter->intersection_count = sem_create("intersection_count", 3);

	return;
}

/*
 * Called by the driver during teardown.
 */

void stoplight_cleanup() {
	lock_destroy(inter->top_left);
	lock_destroy(inter->top_right);
	lock_destroy(inter->bottom_left);
	lock_destroy(inter->bottom_right);

	sem_destroy(inter->intersection_count);

	kfree(inter);

	return;
}

void
turnright(uint32_t direction, uint32_t index)
{
	
	P(inter->intersection_count);

	if (direction == 0) {
		lock_acquire(inter->top_left);
		inQuadrant(direction, index);
		leaveIntersection(index);
		lock_release(inter->top_left);
	} 
	else if (direction == 1) {
		lock_acquire(inter->top_right);
		inQuadrant(direction, index);
		leaveIntersection(index);
		lock_release(inter->top_right);
	}
	else if (direction == 2) {
		lock_acquire(inter->bottom_right);
		inQuadrant(direction, index);
		leaveIntersection(index);
		lock_release(inter->bottom_right);
	}
	else if (direction == 3) {
		lock_acquire(inter->bottom_left);
		inQuadrant(direction, index);
		leaveIntersection(index);
		lock_release(inter->bottom_left);
	}	
		
	V(inter->intersection_count);

	return;
}
void
gostraight(uint32_t direction, uint32_t index)
{
	P(inter->intersection_count);

	if (direction == 0) {
		lock_acquire(inter->top_left);
		inQuadrant(0, index);
		lock_acquire(inter->bottom_left);
		inQuadrant(3, index);
		lock_release(inter->top_left);
		leaveIntersection(index);
		lock_release(inter->bottom_left);
	} 
	else if (direction == 1) {
		lock_acquire(inter->top_right);
		inQuadrant(1, index);
		lock_acquire(inter->top_left);
		inQuadrant(0, index);
		lock_release(inter->top_right);
		leaveIntersection(index);
		lock_release(inter->top_left);
	}
	else if (direction == 2) {
		lock_acquire(inter->bottom_right);
		inQuadrant(2, index);
		lock_acquire(inter->top_right);
		inQuadrant(1, index);
		lock_release(inter->bottom_right);
		leaveIntersection(index);
		lock_release(inter->top_right);
	}
	else if (direction == 3) {
		lock_acquire(inter->bottom_left);
		inQuadrant(3, index);
		lock_acquire(inter->bottom_right);
		inQuadrant(2, index);
		lock_release(inter->bottom_left);
		leaveIntersection(index);
		lock_release(inter->bottom_right);
	}	
		
	V(inter->intersection_count);

	return;
}
void
turnleft(uint32_t direction, uint32_t index)
{
	P(inter->intersection_count);

	if (direction == 0) {
		lock_acquire(inter->top_left);
		inQuadrant(0, index);
		lock_acquire(inter->bottom_left);
		inQuadrant(3, index);
		lock_release(inter->top_left);
		lock_acquire(inter->bottom_right);
		inQuadrant(2, index);
		lock_release(inter->bottom_left);
		leaveIntersection(index);
		lock_release(inter->bottom_right);
	} 
	else if (direction == 1) {
		lock_acquire(inter->top_right);
		inQuadrant(1, index);
		lock_acquire(inter->top_left);
		inQuadrant(0, index);
		lock_release(inter->top_right);
		lock_acquire(inter->bottom_left);
		inQuadrant(3, index);
		lock_release(inter->top_left);
		leaveIntersection(index);
		lock_release(inter->bottom_left);
	}
	else if (direction == 2) {
		lock_acquire(inter->bottom_right);
		inQuadrant(2, index);
		lock_acquire(inter->top_right);
		inQuadrant(1, index);
		lock_release(inter->bottom_right);
		lock_acquire(inter->top_left);
		inQuadrant(0, index);
		lock_release(inter->top_right);
		leaveIntersection(index);
		lock_release(inter->top_left);
	}
	else if (direction == 3) {
		lock_acquire(inter->bottom_left);
		inQuadrant(3, index);
		lock_acquire(inter->bottom_right);
		inQuadrant(2, index);
		lock_release(inter->bottom_left);
		lock_acquire(inter->top_right);
		inQuadrant(1, index);
		lock_release(inter->bottom_right);
		leaveIntersection(index);
		lock_release(inter->top_right);
	}	
		
	V(inter->intersection_count);

	return;
}

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
 * Driver code is in kern/tests/synchprobs.c We will
 * replace that file. This file is yours to modify as you see fit.
 *
 * You should implement your solution to the whalemating problem below.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

/*
 * Called by the driver during initialization.
 */

/*
 * only three whales alowed, but they have to be of a certain type...
 * possibly 3 locks, that will only accept male, female, matchmaker.
 *
 * use of CV to only lock on Male, Female, or Match?
 *
 * if all three are locked then we can mate, otherwsie continue to wait
 *
 */

struct mating *mate;

void whalemating_init() {

	mate = kmalloc(sizeof(struct mating));	

	mate->male_sem = sem_create("male_sem", 0);
	mate->female_sem = sem_create("female_sem", 0);
	mate->male_wait_sem = sem_create("male_wait_sem", 0);
	mate->female_wait_sem = sem_create("female_wait_sem", 0);

	return;
}

/*
 * Called by the driver during teardown.
 */

void
whalemating_cleanup() {

	sem_destroy(mate->male_sem);
	sem_destroy(mate->female_sem);
	sem_destroy(mate->male_wait_sem);
	sem_destroy(mate->female_wait_sem);
	
	kfree(mate);

	return;
}

void
male(uint32_t index)
{
	V(mate->male_sem);

	male_start(index);

	P(mate->male_wait_sem);

	male_end(index);

	return;
}

void
female(uint32_t index)
{
	V(mate->female_sem);

	female_start(index);

	P(mate->female_wait_sem);

	female_end(index);

	return;
}

void
matchmaker(uint32_t index)
{
	P(mate->male_sem);
	P(mate->female_sem);

	matchmaker_start(index);

	V(mate->male_wait_sem);
	V(mate->female_wait_sem);

	matchmaker_end(index);

	return;
}

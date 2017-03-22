/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 *
 * Futex-based mutexes and condition variables.
 * https://locklessinc.com/articles/mutex_cv_futex/
 */

/* ========================================================================== */

#define _GNU_SOURCE

#include <linux/futex.h>

#include <sys/syscall.h>

#include <errno.h>
#include <unistd.h>

#include "../priv.h"

/* -------------------------------------------------------------------------- */

#ifndef __GNUC__
# error we need gcc
#endif

#define atomic_xadd(P, V)	__sync_fetch_and_add((P), (V))
#define cmpxchg(P, O, N)	__sync_val_compare_and_swap((P), (O), (N))
#define atomic_inc(P)		__sync_add_and_fetch((P), 1)
#define atomic_dec(P)		__sync_add_and_fetch((P), -1) 
#define atomic_add(P, V)	__sync_add_and_fetch((P), (V))
#define atomic_set_bit(P, V)	__sync_or_and_fetch((P), 1<<(V))
#define atomic_clear_bit(P, V)	__sync_and_and_fetch((P), ~(1 << (V)))

/* read-write barrier */
#define barrier()		__asm__ __volatile__("": : :"memory")

/* Pause instruction to prevent excess processor bus usage */ 
#define cpu_relax()		__asm__ __volatile__("pause\n": : :"memory")

static inline uint32_t xchg32(void* ptr, uint32_t x)
{
	__asm__ __volatile__("xchgl %0,%1"
				:"=r" ((uint32_t)x)
				:"m" (*(volatile uint32_t*)ptr), "0" (x)
				:"memory");

	return x;
}

static inline uint8_t xchg8(void* ptr, uint8_t x)
{
	__asm__ __volatile__("xchgb %0,%1"
				:"=r" ((uint8_t)x)
				:"m" (*(volatile uint8_t*)ptr), "0" (x)
				:"memory");

	return x;
}

/* -------------------------------------------------------------------------- */

union mutex
{
	uint32_t	u;
	struct {
		uint8_t	locked;
		uint8_t	contended;
	} b;
};

struct cv {
	mutex*	m;
	int	seq;
	int	__pad;
};

/* -------------------------------------------------------------------------- */

static long
sys_futex(void* a1, int op, int v1, struct timespec* to, void* a2, int v3)
{
	return syscall(SYS_futex, a1, op, v1, to, a2, v3);
}

/* -------------------------------------------------------------------------- */

int cond_wait(cv* c, mutex* m)
{
	int s;
	void* t;

	s = c->seq;

	if (m != c->m) {
		/* Atomically set mutex inside cv */
		t = cmpxchg(&c->m, NULL, m);
		if (m != c->m) {
			return EINVAL;
		}
		(void)t;
	}
	
	mutex_unlock(m);
	
	sys_futex(&c->seq, FUTEX_WAIT_PRIVATE, s, 0, 0, 0);
	
	while (0 != (xchg32(&m->b.locked, 257) & 1)) {
		sys_futex(m, FUTEX_WAIT_PRIVATE, 257, 0, 0, 0);
	}
		
	return 0;
}

int cond_signal(cv* c)
{
	/* We are waking someone up */
	atomic_add(&c->seq, 1);
	
	/* Wake up a thread */
	sys_futex(&c->seq, FUTEX_WAKE_PRIVATE, 1, 0, 0, 0);
	
	return 0;
}

/* -------------------------------------------------------------------------- */

int mutex_lock(mutex* m)
{
	size_t i;
	
	/* Try to grab lock */
	for (i = 0 ; 100 > i ; i++) {
		if (0 == xchg8(&m->b.locked, 1)) {
			return 0;
		}

		cpu_relax();
	}

	/* Have to sleep */
	while (0 != (1 & xchg32(&m->u, 257))) {
		sys_futex(m, FUTEX_WAIT_PRIVATE, 257, 0, 0, 0);
	}
	
	return 0;
}

int mutex_unlock(mutex* m)
{
	size_t i;
	
	/* Locked and not contended */
	if ((1 == m->u) && (1 == cmpxchg(&m->u, 1, 0))) {
		return 0;
	}
	
	/* Unlock */
	m->b.locked = 0;
	
	barrier();
	
	/* Spin and hope someone takes the lock */
	for (i = 0 ; 200 > i ; i++) {
		if (0 != m->b.locked) {
			return 0;
		}
		
		cpu_relax();
	}
	
	/* We need to wake someone up */
	m->b.contended = 0;
	
	sys_futex(m, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
	
	return 0;
}

/* ========================================================================== */

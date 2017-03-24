/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 *
 * Semi-dynamic thread pools
 */

/*
 * The tpool module is responsible for the proper management
 * and execution of threads.
 *
 * Threads are not destroyed until the program terminates.
 * If they do not have work, they will remain idle until
 * they are given work.
 */

/*
 * A new pool can steal threads from existing pools.
 * Similarly, stopping a pool can cause other pools to grow.
 */

/* ========================================================================== */

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "../priv.h"

/* -------------------------------------------------------------------------- */

/* A set of actively working threads. */
struct pool {
	size_t		max;

	void		(*func)(void*);
	void*		arg;

	size_t		nthreads;
	struct thread*	threads;
};

struct thread {
	size_t		canrun;
	size_t		running;
	pthread_t	pth;
	struct pool*	pool;

	mutex		lock;
	cv		cond;

	struct thread*	next;
};

/*
 * In the biggest loads, there will be one thread
 * per pool. npools, the number of possible pools
 * is, thus, also the number of threads.
 *
 * njobs is the number of currently active
 * thread pools.
 */
static struct thread* threads;
static struct pool* pools;
static size_t npools;
static size_t njobs;

/* -------------------------------------------------------------------------- */

void* thread_main(void* _thread)
{
	struct thread* th;

	th = (struct thread*)_thread;

	while (1) {
		mutex_lock(&th->lock);
		while (0 == th->canrun) {
			th->running = 0;
			/* signal we're stopped */
			cond_signal(&th->cond);
			cond_wait(&th->cond, &th->lock);
		}
		th->running = 1;
		/* signal we are running */
		cond_signal(&th->cond);
		mutex_unlock(&th->lock);

		//iprintf("thread %x", th);
		th->pool->func(th->pool->arg);
	}

	return 0;
}

/* -------------------------------------------------------------------------- */

static void thread_stop(struct thread* th)
{
	mutex_lock(&th->lock);
	th->canrun = 0;
	while (0 != th->running) {
		cond_wait(&th->cond, &th->lock);
	}
	mutex_unlock(&th->lock);

	th->pool->threads = th->next;
	th->pool->nthreads--;
}

static void thread_start(struct thread* th, struct pool* pl)
{
	th->pool = pl;
	th->next = pl->threads;
	pl->threads = th;
	pl->nthreads++;

	mutex_lock(&th->lock);
	th->canrun = 1;
	cond_signal(&th->cond);
	while (0 == th->running) {
		cond_wait(&th->cond, &th->lock);
	}
	mutex_unlock(&th->lock);
}

/* -------------------------------------------------------------------------- */

/*
 * This function evaluates the current work pools and
 * tries to determine if it should steal threads from
 * an existing pool, or simply use unused threads.
 */

static int __start(size_t pi)
{
	size_t a;
	size_t i;
	struct thread* th;

	a = (npools - njobs);
	gprintf("%zu available threads (asking for %zi)", a, pools[pi].max);

	if (0 != a) {
		gprintf("Assigning as many threads as possible");
		for (i = 0 ; npools > i ; i++) {
			if (pools[pi].nthreads == pools[pi].max) {
				break;
			}
			if (0 == threads[i].running) {
				thread_start(&threads[i], &pools[pi]);
				gprintf("Started thread %x", &threads[i]);
			}
		}
	} else {
		gprintf("Stealing a thread");
		/* steal a thread
		 *
		 * We could try and share the threads equally, but this
		 * would cause synchronization lags and lead to a
		 * complex algorithm. Here, we steal a single thread.
		 */
		for (i = 0 ; npools > i ; i++) {
			if (1 == pools[i].nthreads) {
				continue;
			}

			/* take note of the thread removed */
			th = pools[i].threads;

			thread_stop(pools[i].threads);
			thread_start(th, &pools[i]);
			break;
		}
	}

	njobs++;
	gprintf("Assigned %zu threads", pools[pi].nthreads);

	return pi;
}

int tpool_start(size_t max, void (*func)(void*), void* arg)
{
	size_t p;

	for (p = 0 ; npools > p ; p++) {
		if (0 == pools[p].threads) {
			break;
		}
	}
	if (p == npools) {
		errno = EAGAIN;
		return -1;
	}

	gprintf("Starting thread in pool %zu", p);
	/* init */
	pools[p].max  = max;
	pools[p].func = func;
	pools[p].arg  = arg;
	pools[p].threads = 0;
	pools[p].nthreads = 0;

	return __start(p);
}

/* -------------------------------------------------------------------------- */

/*
 * Share the unassigned threads to new other work pools.
 *
 * This is a best effort. Try to share all inactive
 * thread among all pools.
 *
 * FIXME optimize
 */
static void redistribute_threads(void)
{
	size_t n;
	size_t p;
	size_t t;

	t = 0;

	while (1) {
		n = 0;

		for (p = 0 ; npools > p ; p++) {
			if (0 == pools[p].nthreads) {
				continue;
			} else if (pools[p].nthreads == pools[p].max) {
				continue;
			}

			for (/**/ ; npools > t ; t++) {
				if (0 != threads[t].running) {
					continue;
				}
				thread_start(&threads[t], &pools[p]);
				n++;
				/* no more threads for that pool */
				goto NEXT_POOL;
			}

			/* all threads have been assigned */
			return;
NEXT_POOL:
			/* fix: label at the end of compound statement */
			/* empty */;
		}

		if (0 == n) {
			/* no threads were assigned */
			break;
		}
	}
}

int tpool_stop(int id)
{
	struct thread* th;

	if ((0 > id) || ((size_t)id >= npools)) {
		errno = EINVAL;
		return -1;
	}

	while (0 != (th = pools[id].threads)) {
		gprintf("Stopping thread %x", th);
		thread_stop(th);
		gprintf("Thread %x stopped", th);
	}

	redistribute_threads();

	njobs--;

	return 0;
}

/* -------------------------------------------------------------------------- */

#define ncpus()		(int)sysconf(_SC_NPROCESSORS_ONLN)

static int init_thread(struct thread* th)
{
	th->canrun = 0;
	errno = pthread_create(&th->pth, 0, thread_main, th);

	return (0 == errno) ? 0 : -1;
}

int init_tpool(void)
{
	int c;
	int i;

	c = ncpus();
	gprintf("%u cpus detected", c);
	if ((0 > c) || (TP_MINTHREADS > c)) {
		c = TP_MINTHREADS;
	}
	gprintf("using %u threads", c);

	if (0 == (pools = calloc(c, sizeof(struct pool)))) {
		return -1;
	}
	if (0 == (threads = calloc(c, sizeof(struct thread)))) {
		free(pools);
		return -1;
	}

	for (i = 0 ; c > i ; i++) {
		gprintf("Starting thread %u", i);
		if (0 != init_thread(&threads[i])) {
			return -1;
		}
		npools++;
	}

	if (c != (int)npools) {
		gprintf("Could not initialize all threads (%u>%zu)", c, npools);
		if (TP_MINTHREADS > npools) {
			eprintf("Failed to initialize the minimum amount of threads (%zu)", TP_MINTHREADS);
			return -1;
		}
	}

	return 0;
}

void exit_tpool(void)
{
	size_t i;

	for (i = 0 ; npools > i ; i++) {
		pthread_kill(threads[i].pth, SIGTERM);
		pthread_join(threads[i].pth, 0);
	}

	free(pools);
	free(threads);
}

/* ========================================================================== */

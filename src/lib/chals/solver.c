/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 *
 * Solvers are challenge solvers designed for tpools.
 */

#if 0
Solvers were introduced to reduce the need for memory
allocations during mining periods. When created, they
allocate all the memory they expect to need and, thus,
eliminate allocations. However, a solver is free to
expand the buffers at run-time if it requires to.
#endif

/* ========================================================================== */

#include <stdlib.h>
#include <string.h>

#include "chals.h"

/* -------------------------------------------------------------------------- */

static int solver_resize(struct solver* sl, const struct chal* chl)
{
	if (0 != sortlist_resize(sl, chl)) {
		return -1;
	}

	if (0 != shortpath_resize(sl, chl)) {
		return -1;
	}

	return 0;
}

/* -------------------------------------------------------------------------- */
#include <unistd.h>

static int __solve(const struct chal* chl, struct solver* sl, csolver_t cs)
{
	struct uevent ue;

	if (0 != solver_resize(sl, chl)) {
		eeprintf("Failed to resize solver");
		return -1;
	}

	if (0 == cs(chl, sl)) {
		gprintf("Notifying miner");
		UQ_SET(&ue, chl->id, UQFILT_SOLVR, 0, QNOTE_TRIGGER, 0, 0);
		ue.data = (intptr_t)(sl->nonce);
		uevent(sl->uq, &ue, 1, 0, 0, 0);
	}

	return 0;
}

/* -------------------------------------------------------------------------- */

static void __solver_solve(struct solver* sl)
{
        switch (sl->chl->type) {
        case CHAL_SORT:  /* drop */
        case CHAL_RSORT:
		__solve(sl->chl, sl, sortlist);
		return;
	case CHAL_SPATH:
		__solve(sl->chl, sl, shortpath);
		return;
        default:
                break;
        }

        eprintf("[!] Trying to solve unknown challenge (%i)", sl->chl->type);
	exit(1);
}

static void solver_solve(void* _sl)
{
	__solver_solve(_sl);
}

int solver_start(struct solver* slv, size_t num)
{
	return tpool_start(num, solver_solve, slv);
}

int solver_stop(int tp)
{
	return tpool_stop(tp);
}

/* -------------------------------------------------------------------------- */

int solver_init(struct solver* sl)
{
	struct chal c;

	memset(sl, 0, sizeof(*sl));

	/* dummy challenge for resize */
	c.params.sl.nelems	= SL_EXPNUMS;
	c.params.sp.size	= SP_EXPSIZE;

	if (0 != solver_resize(sl, &c)) {
		eprintf("Failed to initialize solver");
		return -1;
	}

	return 0;
}

void solver_free(struct solver* sl)
{
	sortlist_free(sl);
	shortpath_free(sl);
}

/* ========================================================================== */

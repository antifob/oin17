/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 *
 * Challenge solver: sortlist and reverse sortlist
 */

/* ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chals.h"

#if (!defined(SL_QUICKSORT) && !defined(SL_SMOOTHSORT) && !defined(SL_HEAPSORT) && !defined(SL_MERGESORT))
# define SL_MERGESORT
#endif

/* -------------------------------------------------------------------------- */

struct sortlist {
	size_t		max;
	uint64_t*	nums;
	char*		str;
#if defined(SL_MERGESORT)
	uint64_t*	mtmp;
#endif
};

/* -------------------------------------------------------------------------- */

#ifdef SL_QUICKSORT
static int __sort(const void* _a, const void* _b)
{
	uint64_t a = (uint64_t)_a;
	uint64_t b = (uint64_t)_b;

	return ((a < b) ? -1 : ((a > b) ? 1 : 0));
}
static int __rsort(const void* _a, const void* _b)
{
	uint64_t a = (uint64_t)_a;
	uint64_t b = (uint64_t)_b;

	return ((a > b) ? -1 : ((a < b) ? 1 : 0));
}
#endif

static void sort(const struct chal* chl, struct sortlist* sl)
{
	if (CHAL_SORT == chl->type) {
#ifdef SL_QUICKSORT
		qsort(sl->nums, chl->params.sl.nelems, sizeof(uint64_t), __sort);
#elif defined(SL_HEAPSORT)
		heapsort64(sl->nums, chl->params.sl.nelems, 0);
#elif defined(SL_MERGESORT)
		mergesort64(sl->nums, sl->mtmp, chl->params.sl.nelems, 0);
#else
		smoothsort64(sl->nums, chl->params.sl.nelems, 0);
#endif
	} else if (CHAL_RSORT == chl->type) {
#ifdef SL_QUICKSORT
		qsort(sl->nums, chl->params.sl.nelems, sizeof(uint64_t), __rsort);
#elif defined(SL_HEAPSORT)
		heapsort64(sl->nums, chl->params.sl.nelems, 1);
#elif defined(SL_MERGESORT)
		mergesort64(sl->nums, sl->mtmp, chl->params.sl.nelems, 1);
#else
		smoothsort64(sl->nums, chl->params.sl.nelems, 1);
#endif
	} else {
		eprintf("Internal error: unexpected challenge in sortlist (%llu)", chl->type);
		exit(1);
	}
}

/* -------------------------------------------------------------------------- */

int sortlist(const struct chal* chl, struct solver* slv)
{
	size_t l;
	struct sortlist* sl;
	uint8_t d[SHA256_LEN];
	char lh[SHA256_DLEN + U64STRLEN + 1];


	sl = (struct sortlist*)slv->sl;


	/* FIXME don't do memcpy is chl->id is the same as before? */
	memcpy(lh, chl->lhash, SHA256_DLEN);


	/* generate the random numbers */
	slv->nonce = getnonce(slv->nmin, slv->nmax);
	mt64_seed(slv->prng, mkseed(lh, slv->nonce));
	mt64_randn(slv->prng, sl->nums, chl->params.sl.nelems);

	sort(chl, sl);

	/* make the solution */
	l = mku64str(sl->nums, chl->params.sl.nelems, sl->str);
	sha256(sl->str, l, d);

	if (0 == memcmp(d, chl->pfix, chl->plen)) {
#ifdef OIN17_DEBUG
		char dgst[SHA256_DLEN + 1];
		bin2hex(d, sizeof(d), dgst);
		gprintf("Found %llu for %s", slv->nonce, dgst);
#endif
		return 0;
	}

	return -1;
}

/* -------------------------------------------------------------------------- */

int sortlist_resize(struct solver* slv, const struct chal* chl)
{
	void* tmp;
	struct sortlist* sl;

	if (0 == slv->sl) {
		if (0 == (slv->sl = calloc(1, sizeof(*sl)))) {
			return -1;
		}
	}

	sl = (struct sortlist*)slv->sl;
	if (sl->max >= chl->params.sl.nelems) {
		return 0;
	}

	gprintf("Growing sortlist workspace");
	tmp = REALLOC(sl->nums, chl->params.sl.nelems, sizeof(uint64_t));
	if (0 == tmp) {
		eeprintf("Failed to resize sortlist nums");
		return -1;
	}
	sl->nums = tmp;

	/* can hardly add just one nil byte */
	tmp = REALLOC(sl->str, (chl->params.sl.nelems + 1), U64STRLEN);
	if (0 == tmp) {
		eeprintf("Failed to resize sortlist str");
		return -1;
	}
	sl->str = tmp;

#if defined(SL_MERGESORT)
	tmp = REALLOC(sl->mtmp, chl->params.sl.nelems, sizeof(uint64_t));
	if (0 == tmp) {
		eeprintf("Failed to resize sortlist mergesort tmp buf");
		return -1;
	}
	sl->mtmp = tmp;
#endif

	sl->max = chl->params.sl.nelems;
	return 0;
}

void sortlist_free(struct solver* slv)
{
	struct sortlist* sl;

	if (0 != slv->sl) {
		sl = (struct sortlist*)slv->sl;
		if (0 != sl->nums) {
			free(sl->nums);
		}
		if (0 != sl->str) {
			free(sl->str);
		}
#if defined(SL_MERGESORT)
		if (0 != sl->mtmp) {
			free(sl->mtmp);
		}
#endif
		free(sl);
		slv->sl = 0;
	}
}

/* ========================================================================== */

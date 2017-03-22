/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chals.h"

/* -------------------------------------------------------------------------- */
/* TODO we can refine even more */

/* [10^8, 10^9] or between 7 and 8 digits */
//#define __NONCEMIN	1000000
#define __NONCEMIN	3000000
#define __NONCEMAX	10000000

uint64_t getnonce(void)
{
	/*
	 * In theory, nonce can be [0, NONCEMAX];
	 * where NONCEMAX is expected to be the
	 * nonce limit used by the server.
	 *
	 * In practice, they are usually in
	 * [__NONCEMIN, NONCEMAX[.
	 *
	 * We can eliminate useless work by ensuring
	 * the nonce is between these values.
	 */

	uint64_t n;

	do {
		n = (mt64_rand(0) % NONCEMAX);
	} while ((n < __NONCEMIN) || (n > __NONCEMAX));

	return n;
}

/* ========================================================================== */

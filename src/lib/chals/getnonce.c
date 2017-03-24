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

uint64_t getnonce(uint64_t min, uint64_t max)
{
	uint64_t n;
	static uint64_t sn;
       
	n = __sync_add_and_fetch(&sn, CHAL_NONCEINC);
	n = (n % (max - min)) + min;

	//iprintf("nonce=%llu", n);
	return n;
}

/* ========================================================================== */

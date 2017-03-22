/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include "chals.h"

/* -------------------------------------------------------------------------- */

/* Convert an array of u64 to a string */
size_t mku64str(const uint64_t* buf, size_t cnt, char* dst)
{
	size_t i;
	size_t o;

	for (i = 0, o = 0 ; cnt > i ; i++) {
		o += u64str(buf[i], &dst[o]);
	}

	dst[o] = '\0';

	return o;
}

/* ========================================================================== */

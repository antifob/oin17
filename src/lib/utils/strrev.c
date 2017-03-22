/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include <inttypes.h>
#include <string.h>

/* -------------------------------------------------------------------------- */

void strnrev(char* b, size_t l)
{
	size_t i;
	size_t p;

	p = (l / 2);
	l--;

	for (i = 0 ; p > i ; i++) {
		/* swap */
		b[i]     ^= b[l - i];
		b[l - i] ^= b[i];
		b[i]     ^= b[l - i];
	}
}

void strrev(char* buf)
{
	strnrev(buf, strlen(buf));
}

/* ========================================================================== */

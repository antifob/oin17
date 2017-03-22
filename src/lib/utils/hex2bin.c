/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include <stddef.h>
#include <stdint.h>

/* -------------------------------------------------------------------------- */

/* convert an hex character to its binary representation */
/* NOTE Does not validate the input */
static char x2b(char c)
{
	if (c > '9') {
                c += 9;
        }
	return c & 15;
}

size_t hex2bin(const char* str, void* _dst)
{
	size_t i;
	size_t j;
	uint8_t* d;

	d = (uint8_t*)_dst;

	for (i = 0, j = 0 ; '\0' != str[i] ; i += 2, j++) {
		if (0 != d) {
			d[j] = (x2b(str[i]) << 4) | x2b(str[i + 1]);
		}
	}

	return j;
}

/* ========================================================================== */

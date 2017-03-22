/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include <stddef.h>
#include <stdint.h>

/* -------------------------------------------------------------------------- */

static char b2x(char c)
{
	static const char x[] = "0123456789abcdef";
	return x[c % 16];
}

size_t bin2hex(const void* buf, size_t len, char* dst)
{
	size_t i;
	size_t j;
	uint8_t* p;

	p = (uint8_t*)buf;

	for (i = 0, j = 0 ; len > i ; i++, j += 2) {
		if (0 != dst) {
			dst[j]     = b2x((p[i] & 0xf0) >> 4);
			dst[j + 1] = b2x(p[i] & 0x0f);
		}
	}

	if (0 != dst) {
		dst[j] = '\0';
	}

	return j;
}

/* ========================================================================== */

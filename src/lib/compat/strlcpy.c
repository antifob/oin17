/*
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include <stddef.h>

size_t strlcpy(char *dst, const char *src, size_t dlen)
{
	size_t i;

	i = 0;

	if (0 != dlen) {
		dlen--;
		for (i = 0 ; dlen > i ; i++) {
			dst[i] = src[i];
			if ('\0' == src[i]) {
				break;
			}
		}
	}

	while ('\0' != src[i]) {
		i++;
	}

	return i;
}

/* -------------------------------------------------------------------------- */

#if defined(STRLCPY_TEST)

#include <stdio.h>

/* the destination buffer's size */
#define TESTLEN	2

static void test(const char* s)
{
	size_t l;
	char d[TESTLEN];

	d[0] = 0;
	l = strlcpy(d, s, sizeof(d));
	printf("'%s'->'%s' %i\n", s, d, l);
}

int main(void)
{
	size_t i;
	char b[TESTLEN + 2];

	for (i = 0 ; sizeof(b) > i ; i++) {
		b[i] = '\0';
		test(b);
		b[i] = 'a';
	}

	return 0;
}

#endif /* STRLCPY_TEST */

/* ========================================================================== */

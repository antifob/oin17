/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 *
 * Adapted version of (2017-03-15):
 * https://en.wikibooks.org/wiki/Algorithm_Implementation/Sorting/Smoothsort
 */

/* ========================================================================== */

#include <stddef.h>
#include <stdint.h>

/* -------------------------------------------------------------------------- */

void heapsort64(uint64_t* buf, size_t cnt, size_t rev)
{
#define __cmp(A, B, R)	((0 != R) ? (A > B) : (A <= B))

	size_t c;
	size_t i;
	size_t p;
	uint64_t t;

	p = (cnt / 2);

	while (0 != 1) {
		if (0 < p) {
			t = buf[--p];
		} else {
			if (0 == --cnt) {
				return;
			}
			t = buf[cnt];
			buf[cnt] = buf[0];
		}

		i = p;
		c = ((i * 2) + 1);
		while (cnt > c) {
			if (cnt > (c + 1)) {
				if (0 != rev) {
					if (buf[(c + 1)] < buf[c]) {
						c++;
					}
				} else {
					if (buf[(c + 1)] > buf[c]) {
						c++;
					}
				}
			}
			if (__cmp(buf[c], t, rev)) {
				break;
			}
			buf[i] = buf[c];
			i = c;
			c = (i * 2 + 1);
		}

		buf[i] = t;
	}
}

/* -------------------------------------------------------------------------- */

#ifdef HEAPSORT64_MAIN
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static uint64_t now(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ((uint64_t)ts.tv_sec * 1000000000) + ts.tv_nsec;
}

int main(void)
{
#define N(a)	(sizeof(a) / sizeof(a[0]))

	uint64_t a[1000];

	size_t i;
	uint64_t t1;
	uint64_t t2;

	for (i = 0 ; N(a) > i ; i++) a[i] = rand();

	t1 = now();
	heapsort64(a, N(a), 0);
	t2 = now();
	//for (i = 0 ; N(a) > i ; i++) printf("%llu\n", a[i]);
	printf("time: %llu\n", t2 - t1);

	for (i = 0 ; N(a) > i ; i++) a[i] = rand();
	t1 = now();
	heapsort64(a, N(a), 1);
	t2 = now();
	//for (i = 0 ; N(a) > i ; i++) printf("%llu\n", a[i]);
	printf("time: %llu\n", t2 - t1);

	return 0;
}
#endif /* HEAPSORT64_MAIN */

/* ========================================================================== */

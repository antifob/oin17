/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 *
 * Bottom-up merge sort
 *
 * Adapted version of (2017-03-23):
 * https://en.wikibooks.org/wiki/Algorithm_Implementation/Sorting/Mergesort
 */

/* ========================================================================== */

#include <stddef.h>
#include <stdint.h>

/* -------------------------------------------------------------------------- */

static void
merge(uint64_t* src, uint64_t* dst, size_t l, size_t r, size_t e, size_t rev)
{
#define __cmp(A, B, R)	((0 != R) ? (A > B) : (A < B))

	size_t i;
	size_t j;
	size_t k;

	i = l;
	j = r;

	for (k = l ; e > k ; k++) {
		if ((r > i) && ((e <= j) || __cmp(src[i], src[j], rev))) {
			dst[k] = src[i++];
		} else {
			dst[k] = src[j++];
		}
	}
}

void mergesort64(uint64_t* src, uint64_t* tmp, size_t cnt, size_t rev)
{
#define min(a, b)	(((a) < (b)) ? (a) : (b))
	size_t i;
	size_t j;

	for (i = 1 ; cnt > i ; i = (2 * i)) {
		for (j = 0 ; cnt > j ; j += (2 * i)) {
			merge(src, tmp, j, min((j + i), cnt), min((j + (2 * i)), cnt), rev);
		}
		for (j = 0 ; cnt > j ; j++) {
			src[j] = tmp[j];
		}
	}
}

/* -------------------------------------------------------------------------- */

#ifdef MERGESORT64_MAIN
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static uint64_t now(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ((uint64_t)ts.tv_sec * 1000000000) + ts.tv_nsec;
}

static void __sort(uint64_t* buf, uint64_t* tmp, size_t cnt, size_t rev)
{
	size_t i;
	uint64_t a;
	uint64_t b;

	a = now();
	mergesort64(buf, tmp, cnt, rev);
	b = now();

	for (i = 0 ; cnt > i ; i++) {
		printf("%llu\n", buf[i]);
	}

	//printf("time: %llu\n", (b - a));
}

static void sort(uint64_t* buf, uint64_t* tmp, size_t cnt, size_t rev)
{
	size_t i;

	for (i = 0 ; cnt > i ; i++) {
		buf[i] = rand();
	}

	__sort(buf, tmp, cnt, rev);
	//__sort(buf, tmp, cnt, rev);
	//__sort(buf, tmp, cnt, !rev);
	//__sort(buf, tmp, cnt, !rev);
}

int main(void)
{
#define N(a)	(sizeof(a) / sizeof(a[0]))

	uint64_t a[1000];
	uint64_t b[N(a)];

	//sort(a, b, N(a), 0);
	sort(a, b, N(a), 1);

	return 0;
}
#endif /* MERGESORT64_MAIN */

/* ========================================================================== */

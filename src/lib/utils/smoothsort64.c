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

static inline int __cmp(uint64_t a, uint64_t b)
{
	return ((a < b) ? -1 : ((a == b) ? 0 : 1));
}
static inline int cmp(uint64_t a, uint64_t b, size_t rev)
{
	return ((0 != rev) ? (0 - __cmp(a, b)) : __cmp(a, b));
}

/* -------------------------------------------------------------------------- */

struct state {
	uint64_t*	a;
	size_t		v;	/* reverse */
	size_t		q;
	size_t		r;
	size_t		p;
	size_t		b;
	size_t		c;
	size_t		r1;
	size_t		b1;
	size_t		c1;
};

/* -------------------------------------------------------------------------- */

static inline void up(size_t* a, size_t* b)
{
	size_t t;

	t  = *a;
	*a = (*a + *b + 1);
	*b = t;
}

static inline void down(size_t* a, size_t* b)
{
	size_t t;

	t  = *b;
	*b = (*a - *b - 1);
	*a = t;
}

static void sift(struct state* s)
{
	size_t r0;
	size_t r2;
	uint64_t t;


	r0 = s->r1;
	t  = s->a[r0];


	while (2 < s->b1) {
		r2 = (s->r1 - s->b1 + s->c1);

		if (0 <= cmp(s->a[s->r1 - 1], s->a[r2], s->v)) {
			r2 = (s->r1 - 1);
			down(&s->b1, &s->c1);
		}

		if (0 > cmp(s->a[r2], t, s->v)) {
			s->b1 = 1;
		} else {
			s->a[s->r1] = s->a[r2];
			s->r1 = r2;
			down(&s->b1, &s->c1);
		}
	}

	if (0 < (s->r1 - r0)) {
		s->a[s->r1] = t;
	}
}

static void trinkle(struct state* s)
{
	size_t p1;
	size_t r0;
	size_t r2;
	size_t r3;
	uint64_t t;


	p1    = s->p;
	s->b1 = s->b;
	s->c1 = s->c;
	r0    = s->r1; 
	t     = s->a[r0];


	while (0 < p1) {
		while (0 == (p1 & 1)) {
			p1 = (p1 >> 1);
			up(&s->b1, &s->c1);
		}


		r3 = (s->r1 - s->b1);

		if ((1 == p1) || (0 > cmp(s->a[r3], t, s->v))) {
			p1 = 0;
		} else {
			p1--;

			if (1 == s->b1) {
				s->a[s->r1] = s->a[r3];
				s->r1 = r3;
			} else if (3 <= s->b1) {
				r2 = (s->r1 - s->b1 + s->c1);

				if (0 <= cmp(s->a[s->r1 - 1], s->a[r2], s->v)) {
					r2 = (s->r1 - 1);
					down(&s->b1, &s->c1);
					p1 = (p1 << 1);
				}

				if (0 > cmp(s->a[r2], s->a[r3], s->v)) {
					s->a[s->r1] = s->a[r3];
					s->r1 = r3;
				} else {
					s->a[s->r1] = s->a[r2];
					s->r1 = r2;
					down(&s->b1, &s->c1);
					p1 = 0;
				}
			}
		}
	}

	if (0 != (s->r1 - r0)) {
		s->a[s->r1] = t;
	}

	sift(s);
}

static void semitrinkle(struct state* s)
{
	s->r1 = (s->r - s->c);

	if (0 <= cmp(s->a[s->r1], s->a[s->r], s->v)) {
		/* swap */
		s->a[s->r]  ^= s->a[s->r1];
		s->a[s->r1] ^= s->a[s->r];
		s->a[s->r]  ^= s->a[s->r1];

		trinkle(s);
	}
}

/* -------------------------------------------------------------------------- */

void smoothsort64(uint64_t* a, size_t n, size_t rev)
{
	size_t t;
	struct state s;


	s.a = a;
	s.v = rev;
	s.r = 0;
	s.p = 1;
	s.b = 1;
	s.c = 1;


	/* build tree */
	for (s.q = 1 ; n > s.q ; s.q++) {
		s.r1 = s.r;
		if (3 == (s.p & 7)) {
			s.b1 = s.b;
			s.c1 = s.c;
			sift(&s);

			s.p = ((s.p + 1) >> 2);

			t   = (s.b + s.c + 1);
			s.b = (s.b + t + 1);
			s.c = t;
		} else if (1 == (s.p & 3)) {
			if (n > (s.q + s.c)) {
				s.b1 = s.b;
				s.c1 = s.c;
				sift(&s);
			} else {
				trinkle(&s);
			}

			do {
				down(&s.b, &s.c);
				s.p  = (s.p << 1);
			} while (1 < s.b);

			s.p++;
		}

		s.r++;
	}

	s.r1 = s.r;
	trinkle(&s);


	/* sort */
	while (1 < s.q--) {
		if (1 == s.b) {
			s.r--;
			s.p--;

			while (0 == (s.p & 1)) {
				s.p = (s.p >> 1);
				up(&s.b, &s.c);
			}
		} else if (2 < s.b) {
			s.p--;
			s.r = (s.r - s.b + s.c);

			if (0 < s.p) {
				semitrinkle(&s);
			}
			down(&s.b, &s.c);

			s.p = ((s.p << 1) + 1);
			s.r = (s.r + s.c);
			semitrinkle(&s);
			down(&s.b, &s.c);

			s.p = ((s.p << 1) + 1);
		}
	}
}

/* -------------------------------------------------------------------------- */

#ifdef SMOOTHSORT64_MAIN
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
#define N(a)	(sizeof(a) / sizeof(a[0]))

	uint64_t a[1000];

	size_t i;

	for (i = 0 ; N(a) > i ; i++) a[i] = rand();
	smoothsort64(a, N(a), 0);
	for (i = 0 ; N(a) > i ; i++) printf("%llu\n", a[i]);
	for (i = 0 ; N(a) > i ; i++) a[i] = rand();
	smoothsort64(a, N(a), 1);
	for (i = 0 ; N(a) > i ; i++) printf("%llu\n", a[i]);

	return 0;
}
#endif /* SMOOTHSORT64_MAIN */

/* ========================================================================== */

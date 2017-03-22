/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 *
 * Mersenne Twister 64
 *
 * This is a Mersenne Twister 64 implementation with
 * supports for independent PRNGs. It allows us to
 * use multiple solving threads without having to
 * worry about a thread seeding the PRNG while others
 * are generating numbers.
 *
 * There is also a shared PRNG; which should only be
 * seeded during initialization.
 *
 * The implementation has been tested against the
 * official implementation. See tests/.
 */

/* ========================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "../priv.h"

/* -------------------------------------------------------------------------- */

#ifndef MT64_NN
# define MT64_NN	312
#elif 312 != MT64_NN
# error MT64_NN is set to an invalid value
#endif

#define MT64_MM		156
#define MT64_UM		0xFFFFFFFF80000000ULL	/* MSB (33) */
#define MT64_LM		0x000000007FFFFFFFULL	/* LSB (31) */

static const uint64_t mag01[2] = { 0, 0xb5026f5aa96619e9ULL };

/* -------------------------------------------------------------------------- */

/* shared/default state */
static struct mt64 gmt = { .mti = (MT64_NN + 1) };


static void __mt64_seed(struct mt64* mt, uint64_t seed)
{
#define __MT64_SEEDMUL	6364136223846793005ULL

	size_t i;
	uint64_t* p;

	p = mt->mt;
	p[0] = seed;

	for (i = 1 ; MT64_NN > i ; i++) {
		p[i] = (__MT64_SEEDMUL * (p[i - 1] ^ (p[i - 1] >> 62)) + i);
	}

	mt->mti = i;
}

void mt64_seed(struct mt64* mt, uint64_t seed)
{
	if (0 == mt) {
		mt = &gmt;
	}
	__mt64_seed(mt, seed);
}

/* -------------------------------------------------------------------------- */

static uint64_t __mt64_rand(struct mt64* mt)
{
	size_t i;
	uint64_t x;
	uint64_t* p;

	p = mt->mt;

	if (MT64_NN <= mt->mti) {
		if ((MT64_NN + 1) == mt->mti) {
			mt64_seed(mt, 5489ULL);
		}

		for (i = 0 ; (MT64_NN - MT64_MM) > i ; i++) {
			x = (p[i] & MT64_UM) | (p[i + 1] & MT64_LM);
			p[i] = p[i + MT64_MM] ^ (x >> 1) ^ mag01[(x & 1)];
		}

		for (/**/ ; (MT64_NN - 1) > i ; i++) {
			x = (p[i] & MT64_UM) | (p[i + 1] & MT64_LM);
			p[i] = p[i + (MT64_MM - MT64_NN)] ^ (x >> 1) ^ mag01[(x & 1)];
		}

		x = (p[(MT64_NN - 1)] & MT64_UM) | (p[0] & MT64_LM);
		p[(MT64_NN - 1)] = p[(MT64_MM - 1)] ^ (x >> 1) ^ mag01[(x & 1)];

		mt->mti = 0;
	}

	x  = p[mt->mti++];
	x ^= (x >> 29) & 0x5555555555555555ULL;
	x ^= (x << 17) & 0x71D67FFFEDA60000ULL;
	x ^= (x << 37) & 0xFFF7EEE000000000ULL;
	x ^= (x >> 43);

	return x;
}

uint64_t mt64_rand(struct mt64* mt)
{
	if (0 == mt) {
		mt = &gmt;
	}
	return __mt64_rand(mt);
}

void mt64_randn(struct mt64* mt, uint64_t* buf, size_t len)
{
	size_t i;

	for (i = 0 ; len > i ; i++) {
		buf[i] = mt64_rand(mt);
	}
}

/* ========================================================================== */

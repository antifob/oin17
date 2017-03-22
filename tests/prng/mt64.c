
#include <stdint.h>
#include <time.h>

#define MT64_NN 312

/* -------------------------------------------------------------------------- */

/* constants */
#ifndef MT64_NN
# error MT64_NN not defined
#endif
#define MT64_MM	156
#define MT64_UM	0xFFFFFFFF80000000ULL	/* MSB (33) */
#define MT64_LM	0x000000007FFFFFFFULL	/* LSB (31) */

static const uint64_t mag01[2] = { 0, 0xb5026f5aa96619e9ULL };

/* -------------------------------------------------------------------------- */

struct mt64 {
	uint64_t	mt[MT64_NN];
	size_t		mti;
};

static struct mt64 gmt = { .mti = (MT64_NN + 1) };

static void __mt64_seed(struct mt64* mt, uint64_t seed)
{
	size_t i;
	uint64_t* p;

	p = mt->mt;
	mt->mt[0] = seed;

	for (i = 1 ; MT64_NN > i ; i++) {
		mt->mt[i] = (6364136223846793005ULL * (mt->mt[i - 1] ^ (mt->mt[i - 1] >> 62)) + i);
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

	x = p[mt->mti++];

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

int main(int argc, char* const* argv)
{
#include <stdio.h>
#include <string.h>

	struct mt64 mt;

	if (1 == argc) {
		fprintf(stderr, "usage: %s <+num>\n", argv[0]);
		return 1;
	}

	mt64_seed(&mt, strtoll(argv[1], 0, 10));

	for (argc = 0 ; NTESTS > argc ; argc++) {
		printf("%llu\n", mt64_rand(&mt));
	}

	return 0;
}

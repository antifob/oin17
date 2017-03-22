/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 *
 * PRNG seed generator
 *
 * - combine a hash and a nonce
 * - sha256 on it
 * - return the first 8 bytes as an u64
 */

/* ========================================================================== */

#include "chals.h"

/* -------------------------------------------------------------------------- */

uint64_t mkseed(char* hbuf, uint64_t non)
{
	size_t l;
	uint64_t* p;
	uint8_t sha[SHA256_LEN];

	p = (uint64_t*)sha;

	hbuf[SHA256_DLEN] = 0;
	l = u64str(non, &hbuf[SHA256_DLEN]);
	hbuf[(SHA256_DLEN + l)] = 0;
	sha256(hbuf, (SHA256_DLEN + l), sha);

	/* FIXME little-endian only? */
	return p[0];
}

/* ========================================================================== */

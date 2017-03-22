/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include <stdint.h>

#include <openssl/sha.h>

/* -------------------------------------------------------------------------- */

void sha256(const void* dat, size_t len, uint8_t* dgst)
{
	SHA256_CTX c;

	SHA256_Init(&c);
	SHA256_Update(&c, dat, len);
	SHA256_Final(dgst, &c);
}

/* ========================================================================== */

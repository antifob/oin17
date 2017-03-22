/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 *
 * Wallet manipulation
 */

/* ========================================================================== */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <openssl/rsa.h>
#include <openssl/x509.h>

#include "priv.h"

/* -------------------------------------------------------------------------- */

int wallet_sign(struct wallet* wl, const void* msg, size_t len)
{
	/*
	 * FIXME
	 * We should not use a static buffer as, technically,
	 * the signature length can vary. In practice, it is
	 * constant for 1024-bits RSA keys.
	 */
	uint8_t b[512];

	gprintf("Signing %zu bytes with wallet %s", len, wl->id);


	if (0 > rsa_sign(msg, len, wl->keys, b)) {
		eprintf("Failed to sign message");
		return -1;
	}

	/* Signatures are sent hex-encoded. */
	bin2hex(b, rsa_signlen(wl->keys), wl->lastsig);

	return 0;
}

/* -------------------------------------------------------------------------- */

int wallet_save(const struct wallet* wl, const char* path)
{
	return rsa_savekeys(wl->keys, path);
}

/* -------------------------------------------------------------------------- */

/*
 * https://www.openssl.org/docs/man1.1.0/crypto/i2d_RSA_PUBKEY.html
 */
static int makewid(struct wallet* wl)
{
	size_t l;
	uint8_t* p;
	uint8_t d[SHA256_LEN];

	gprintf("Computing wallet id");


	if (0 == (p = rsa_toder(wl->keys, &l))) {
		gprintf("Failed to convert RSA to DER");
		return -1;
	}

	sha256(p, l, d);
	bin2hex(d, sizeof(d), wl->id);

	free(p);

	return 0;
}

/* prepares the wallet signature buffer */
static int __prepwallet(struct wallet* wl)
{
	wl->signlen = rsa_signlen(wl->keys);
	wl->signlen = (wl->signlen * 2) + 1;	/* hex+nil */

	/* FIXME calloc? */
	if (0 != (wl->lastsig = calloc(1, wl->signlen))) {
		if (0 == makewid(wl)) {
			return 0;
		}
		free(wl->lastsig);
	}

	return -1;
}

int wallet_load(struct wallet* wl, const char* path)
{
	gprintf("Loading wallet at '%s'", path);


	if (0 != (wl->keys = rsa_loadkeys(path))) {
		if (0 == __prepwallet(wl)) {
			return 0;
		}
		/* TODO rsa_destroy(wl->keys); */
	}

	memset(wl, 0, sizeof(*wl));

	return -1;
}

int wallet_loadmem(struct wallet* wl, const void* buf, size_t len)
{
	gprintf("Loading wallet from memory");

	if (0 != (wl->keys = rsa_loadmemkeys(buf, len))) {
		if (0 == __prepwallet(wl)) {
			return 0;
		}
		/* TODO rsa_destroy(wl->keys); */
	}

	memset(wl, 0, sizeof(*wl));

	return -1;
}

/* -------------------------------------------------------------------------- */

int wallet_new(struct wallet* wl)
{
	gprintf("Creating new wallet");


	if (0 != (wl->keys = rsa_keygen())) {
		if (0 == __prepwallet(wl)) {
			return 0;
		}
		/* TODO rsa_destroy(wl->keys); */
	}

	memset(wl, 0, sizeof(*wl));

	return -1;
}

void wallet_free(struct wallet* wl)
{
	(void)wl; /* tmp */
	/* TODO rsa_destroy(wl->keys) */
	/* FIXME segfault on free(lastsig) */
	//free(wl->lastsig);
}

/* ========================================================================== */

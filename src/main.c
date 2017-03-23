/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "oin17.h"
#include "key.h"

/* -------------------------------------------------------------------------- */

static struct wallet wallet;

/* -------------------------------------------------------------------------- */

static int init_wallet(void)
{
	if (0 != wallet_loadmem(&wallet, walletkey, (sizeof(walletkey) - 1))) {
		eprintf("Failed to load wallet");
		return -1;
	}

#ifndef WALLET_REGISTERED
	if (0 != access(".registered", F_OK)) {
		if (0 != register_wallet(&wallet)) {
			wallet_free(&wallet);
			return -1;
		}

		iprintf("Wallet was registered succesfully");

		/*
		 * Create a flag file, in case we run again
		 * without having exited the container.
		 */
		FILE* f;
		if (0 != (f = fopen(".registered", "w"))) {
			fclose(f);
		}
	}
#endif

	iprintf("Using registered wallet: %s", wallet.id);

	return 0;
}

/* -------------------------------------------------------------------------- */

static int init(const char* uri)
{
	char u[256];

	strcpy(u, uri);
	if (0 != init_lib(u)) {
		return -1;
	}
	if (0 != init_wallet()) {
		return -1;
	}
	if (0 != init_mining()) {
		return -1;
	}

	return 0;
}

static void deinit(void)
{
	exit_mining();
	wallet_free(&wallet);
	exit_lib();
}

/* -------------------------------------------------------------------------- */

int main(void)
{
	int r;

	setbuf(stdout, 0);
	setbuf(stderr, 0);

	r = 1;
	if (0 == init(CA_URI)) {
		r = mine(&wallet);
	}
	deinit();

	return r;
}

/* ========================================================================== */

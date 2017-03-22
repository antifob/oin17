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

/* -------------------------------------------------------------------------- */

static struct wallet wallet;

/* -------------------------------------------------------------------------- */

static int init_wallet(void)
{
	if (0 == wallet_load(&wallet, WALLET_PATH)) {
		iprintf("Loaded wallet %s", wallet.id);
		return 0;
	}

	iprintf("Creating wallet");
	if (0 != wallet_new(&wallet)) {
		return -1;
	}
	if (0 != register_wallet(&wallet)) {
		wallet_free(&wallet);
		return -1;
	}
	if (0 != wallet_save(&wallet, WALLET_PATH)) {
		eprintf("Could not save wallet");
		return -1;
	}

	iprintf("Wallet saved at '%s'", WALLET_PATH);

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

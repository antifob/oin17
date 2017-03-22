/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include <unistd.h>

#include "priv.h"

/* -------------------------------------------------------------------------- */

static int init_threads(void)
{
	gprintf("Starting threads");

	if (0 != tpool_start(1, ws_dispatch, 0)) {
		eprintf("Failed to start WebSockets thread");
		return -1;
	}

	return 0;
}

/* -------------------------------------------------------------------------- */

int init_lib(char* uri)
{
	mt64_seed(0, time(0) ^ getpid());

	if (0 != init_tpool()) {
		return -1;
	}
	if (0 != init_threads()) {
		return -1;
	}
	if (0 != init_ws(uri)) {
		return -1;
	}

	return 0;
}

void exit_lib(void)
{
	exit_ws();
	exit_tpool();
}

/* ========================================================================== */

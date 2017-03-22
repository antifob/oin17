/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include "cmd.h"

/* -------------------------------------------------------------------------- */

int ca_curchal(int sk)
{
	const char c[] = "{\"command\":\"get_current_challenge\",\"args\":{}}";

	SENDRETURN(sk, c, (sizeof(c)-1));
}

/* ========================================================================== */

/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include "cmd.h"

/* -------------------------------------------------------------------------- */

int ca_info(int sk)
{
	const char c[] = "{\"command\":\"ca_server_info\",\"args\":{}}";

	SENDRETURN(sk, c, (sizeof(c)-1));
}

/* ========================================================================== */

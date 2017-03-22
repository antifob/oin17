/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include "cmd.h"

/* -------------------------------------------------------------------------- */

/*
 * The submission command is conflicting. See: coinslib
 * versus caserver.
 *
 * TODO Prepare for both format.
 *
 * caserver and the specification indicates it has two (2)
 * arguments: nonce and wallet_id.
 *
 * coinslib indicates that it has four (4) arguments:
 * challenge_id, nonce, hash, signature. The signature
 * is a string composed of the three (3) first arguments
 * joined with ',', signed with the wallet's private key
 * and converted to hexadecimal.
 */


int ca_submit(int sk, const struct wallet* wl, uint64_t non)
{
	const char* f = MAKE_CMDFMT("submission",
			"\"wallet_id\":\"%s\",\"nonce\":%" PRIu64);

	return __sendcmd(sk, f, wl->id, non);
}

/* ========================================================================== */

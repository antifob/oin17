/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include <stdlib.h>

#include "cmd.h"

/* -------------------------------------------------------------------------- */

/*
 * Convert newlines ('\n' to "\\n")
 *
 * The CA does not like control characters.
 * The original buffer pointer may be modified.
 * If it is, the original memory is freed.
 *
 * Returns 0 on success.
 *
 * Since registration occurs before mining
 * begins, we can disregard dynamic memory usage.
 */
static int pem_expand(char** pem)
{
	char* p;
	size_t i;
	size_t j;


	for (i = 0, j = 0 ; '\0' != (*pem)[i] ; i++) {
		if ('\n' == (*pem)[i]) {
			j++;
		}
	}

	if (0 == (p = malloc(i + j + 1))) {
		return -1;
	}

	for (i = 0, j = 0 ; '\0' != (*pem)[i] ; i++, j++) {
		if ('\n' == (*pem)[i]) {
			p[j] = '\\';
			j++;
			p[j] = 'n';
		} else {
			p[j] = (*pem)[i];
		}
	}

	p[j] = '\0';

	free(*pem);
	*pem = p;

	return 0;
}

/* -------------------------------------------------------------------------- */

#ifndef TEAM_NAME
# define TEAM_NAME
#endif

static const char* fmt =
	MAKE_CMDFMT("register_wallet",
	"\"name\":\"" TEAM_NAME "\",\"key\":\"%s\",\"signature\":\"%s\"");


int ca_register(int sk, struct wallet* wl)
{
	int r;
	size_t l;
	char* pem;
	uint8_t* der;
	uint8_t sha[SHA256_LEN];

	/*
	 * name: freeform
	 * key: pem-encoded public key
	 * signature: hex(sign(sha256(der(pubkey))))
	 */
	/* FIXME can we make rsa_toder use a static buffer? */

	r = -1;
	if (0 != (pem = rsa_topem(wl->keys))) {
		pem_expand(&pem);

		if (0 != (der = rsa_toder(wl->keys, &l))) {
			sha256(der, l, sha);

			/* wallet_sign takes care of hex-encoding */
			if (0 == wallet_sign(wl, sha, sizeof(sha))) {
				r = __sendcmd(sk, fmt, pem, wl->lastsig);
			}
			free(der);
		}
		free(pem);
	}

	return r;
}

/* ========================================================================== */

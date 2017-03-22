/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <jsmn.h>

#include "chals.h"

/* -------------------------------------------------------------------------- */

static int __parse_toks(const char* key, const char* val, struct chal* chal)
{
	/*
	 * The CA's responses are generally out of order.
	 * There is no point in trying to minimize the
	 * number of comparisons.
	 */
	gprintf("tok=%s,%s", key, val);


	if (0 == strcmp("time_left", key)) {
		chal->left = strtol(val, 0, 10);
	} else if (0 == strcmp("nb_elements", key)) {
		chal->params.sl.nelems = strtol(val, 0, 10);
	} else if (0 == strcmp("grid_size", key)) {
		chal->params.sp.size = strtol(val, 0, 10);
	} else if (0 == strcmp("nb_blockers", key)) {
		chal->params.sp.nblks = strtol(val, 0, 10);
	} else if (0 == strcmp("challenge_name", key)) {
		if (0 == strcmp("sorted_list", val)) {
			chal->type = CHAL_SORT;
		} else if (0 == strcmp("reverse_sorted_list", val)) {
			chal->type = CHAL_RSORT;
		} else if (0 == strcmp("shortest_path", val)) {
			chal->type = CHAL_SPATH;
		} else {
			/* FIXME keep in sync with challenges */
			eprintf("Unknown challenge: %s", val);
			return -1;
		}
	} else if (0 == strcmp("hash_prefix", key)) {
		/* convert now to accelerate comparison in challenges */
		chal->plen = hex2bin(val, chal->pfix);
	} else if (0 == strcmp("challenge_id", key)) {
		chal->id = strtol(val, 0, 10);
	} else if (0 == strcmp("last_solution_hash", key)) {
		strcpy(chal->lhash, val);
	} else if (0 == strcmp("type", key)) {
		/* ignore */
	} else {
		wprintf("Unexpected key in challenge response: %s", key);
	}

	return 0;
}

static int
parse_toks(const char* buf, const jsmntok_t* toks, size_t ntoks, struct chal* chal)
{
#define tokcpy(dst)	do {							\
	memcpy((dst), &buf[toks[i].start], (toks[i].end - toks[i].start));	\
	(dst)[(toks[i].end - toks[i].start)] = '\0';				\
} while (0)


	int d;
	size_t i;
	char k[64];	/* FIXME longest key */
	char v[256];	/* FIXME longest value */

	/* i = 1, skip the response dict envelope */
	for (i = 1, d = 0 ; ntoks > i ; i++) {
		switch (toks[i].type) {
		case JSMN_STRING:
			if (0 == d) {
				tokcpy(k);
				d = 1;
			} else {
				tokcpy(v);
				d = 2;
			}
			break;
			
		case JSMN_PRIMITIVE:
			tokcpy(v);
			d++;
			break;

		case JSMN_OBJECT:
			/*
			 * Currently, we can forget that parameters is an object
			 * as all parameters children are named differently from
			 * other keys.
			 */
			d = 0;
			break;

		default:
			eprintf("Unexpected token type: %i", toks[i].type);
			return -1;
			break;
		}

		if (2 == d) {
			if (0 != __parse_toks(k, v, chal)) {
				return -1;
			}
			d = 0;
		}
	}

	return 0;
}

/*
 * The maximum number of tokens we can encounter.
 * jsmn counts a key pair as two distinct tokens
 *
 *     1	the response itself
 *     (7*2)	top level tokens
 *     (2*2)	challenge parameters (sortlist, shortpath)
 *              TODO keep in sync with challenges
 */
#define MAX_PARAMS 2
#define MAX_TOKENS (1 + (7 * 2) + (MAX_PARAMS * 2))

int chal_parse(const void* buf, size_t len, struct chal* chal)
{
	int r;
	char* e;
	jsmn_parser p;
	jsmntok_t toks[MAX_TOKENS];

	jsmn_init(&p);

	if (0 > (r = jsmn_parse(&p, buf, len, toks, NARRAY(toks)))) {
		switch (r) {
		case JSMN_ERROR_INVAL: e = "invalid token";   break;
		case JSMN_ERROR_NOMEM: e = "too many tokens"; break;
		case JSMN_ERROR_PART:  e = "partial token";   break;
		default: e = "unknown error"; break;
		}
		eprintf("Failed to parse challenge description (%s)", e);
		return -1;
	}

	return parse_toks(buf, toks, r, chal);
}

/* ========================================================================== */

/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "priv.h"

#define MAXMBUFS	16

/* -------------------------------------------------------------------------- */

static struct mbuf	mbufs[MAXMBUFS];

/* -------------------------------------------------------------------------- */

struct mbuf* mbuf(const void* buf, size_t len)
{
	size_t l;
	static size_t i;


	if (MAXMSGLEN < len) {
		panic("mbufs are too small (MAXMSGLEN < %zu)", len);
	}


	l = i;

	do {
		if (0 == mbufs[i].len) {
			break;
		}
		i++;
		if (MAXMBUFS == i) {
			i = 0;
		}
		if (l == i) {
			panic("not enough mbufs");
		}
	} while (0 != 1);


	memcpy(mbufs[i].buf, buf, len);
	mbufs[i].len = len;
	mbufs[i].next = 0;

	return &mbufs[i];
}

void mbufq(struct mbuf** mb, struct mbuf* el)
{
	struct mbuf* p;

	if (0 == *mb) {
		*mb = el;
	} else {
		p = *mb;
		while (0 != p->next) {
			p = p->next;
		}
		p->next = el;
	}
}

int mbufp(struct mbuf** pmb, void* pbuf, size_t plen)
{
	int r;

	r = 0;
	if (0 != *pmb) {
		if ((*pmb)->len > plen) {
			errno = E2BIG;
			return -1;
		}
		memcpy(pbuf, (*pmb)->buf, (*pmb)->len);
		r = (int)(*pmb)->len;
		(*pmb)->len = 0;

		*pmb = (*pmb)->next;
	}

	return r;
}

/* ========================================================================== */

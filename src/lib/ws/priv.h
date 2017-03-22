/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

#ifndef OIN17_LIBCSCOINS_WS_H
#define OIN17_LIBCSCOINS_WS_H
/* ========================================================================== */

#include "../priv.h"

/* -------------------------------------------------------------------------- */
/* Message buffers
 *
 * mbuf's are used to guarantee that we never send
 * two or more commands in one go to the CA. They
 * are entirely hidden from the application code.
 */

struct mbuf {
	size_t		len;
	char		buf[MAXMSGLEN];
	struct mbuf*	next;
};

/*! mbuf(buf, cnt)
 *
 * Create an mbuf holding a copy of the @cnt
 * first bytes of @buf.
 *
 * Returns nil on error.
 */
extern struct mbuf* mbuf(const void*, size_t);

/*! mbufq(mbq, mb)
 *
 * Enqueue @mb into the @mbq mbuf queue.
 * @mbq is a pointer-pointer to the head
 * of the queue.
 */
extern void mbufq(struct mbuf**, struct mbuf*);

/*! mbufp(mbq, buf, len)
 *
 * Pop off the first mbuf off of @mbq by copying its content
 * into the @buf of length @len.
 *
 * Returns the number of bytes written into @buf.
 * Returns <0 on error (if the buffer is too small).
 */
extern int mbufp(struct mbuf**, void*, size_t);

/* ========================================================================== */
#endif /* !OIN17_WS_H */

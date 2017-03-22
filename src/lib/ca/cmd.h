/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

#ifndef OIN17_CMD_H
#define OIN17_CMD_H
/* ========================================================================== */

#include <stdarg.h>
#include <stdio.h>

#include "../priv.h"

/* -------------------------------------------------------------------------- */

/*
 * Facilitating macro.
 *
 * Try to send @len bytes from @buf through @sk.
 * Return -1 on error and 0 otherwise.
 *
 * MUST terminate a function.
 */
#define SENDRETURN(ws, buf, len)	do {		\
	if ((len) != ws_send((ws), (buf), (len))) {	\
		return -1;				\
	}						\
	return 0;					\
} while (0)

/* -------------------------------------------------------------------------- */

#define MAKE_CMDFMT(c, a)	"{\"command\":\"" c "\",\"args\":{" a "}}"

/*! __sendcmd(ws, fmt, ...)
 *
 * Send a command on @ws by expanding @fmt with the
 * remaining arguments.
 *
 * The MAKE_CMDFMT macro should be used as a facilitator.
 */
static inline int __sendcmd(int sk, const char* fmt, ...)
{
	int n;
	va_list vl;
	char b[4096];

	va_start(vl, fmt);
	n = vsnprintf(b, sizeof(b), fmt, vl);
	va_end(vl);

	if (sizeof(b) <= (size_t)n) {
		eprintf("Command buffer is too small (needed %i+1)", n);
		return -1;
	}
	gprintf("sending cmd: %s", b);

	SENDRETURN(sk, b, n);
}


/* ========================================================================== */
#endif /* !OIN17_CMD_H */

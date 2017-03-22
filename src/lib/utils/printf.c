/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* -------------------------------------------------------------------------- */

void __prsrcinfo(FILE* fp, const char* file, size_t lino, const char* func)
{
	int e = errno;

	fprintf(fp, "%s:%s@%zu ", file, func, lino);

	errno = e;
}

/* -------------------------------------------------------------------------- */

#define MAKE_LPRINTF(name, letter, stream, err)		\
void __ ## name (const char* fmt, ...)			\
{							\
	int e;						\
	va_list vl;					\
							\
	e = errno;					\
	printf("[" # letter "] ");			\
							\
	va_start(vl, fmt);				\
	vfprintf(stream, fmt, vl);			\
	va_end(vl);					\
							\
	if (0 != err) {					\
		fprintf(stderr, ": %s", strerror(e));	\
	}						\
							\
	putchar('\n');					\
	errno = e;					\
}

#if defined(OIN17_DEBUG)
MAKE_LPRINTF(gprintf,  D, stdout, 0)
#else /* !OIN17_DEBUG */
void __gprintf(const char* _, ...)
{
	(void)_;
}
#endif /* OIN17_DEBUG */


MAKE_LPRINTF(iprintf,  I, stdout, 0)
MAKE_LPRINTF(wprintf,  W, stdout, 0)
MAKE_LPRINTF(eprintf,  E, stderr, 0)
MAKE_LPRINTF(eeprintf, E, stderr, 1)

/* ========================================================================== */

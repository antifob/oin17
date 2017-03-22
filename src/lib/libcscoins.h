/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

#ifndef LIBCSCOINS_H
#define LIBCSCOINS_H
/* ========================================================================== */

#include <stdio.h>
#include <uqueue.h>

#include "config.h"

/* -------------------------------------------------------------------------- */

#ifndef SHA256_LEN
# define SHA256_LEN	32		/* (256 / 8) */
#endif
#ifndef SHA256_DLEN
# define SHA256_DLEN	(SHA256_LEN * 2)
#endif


#define UQFILT_WSOCK	(UQFILT_EXTENDMIN + 0)
#define QNOTE_CLOSED	0x00000001	/* connection was closed */
#define QNOTE_CONN	0x00000002	/* connection was established */
#define QNOTE_READ	0x00000004	/* data available */
#define UQFILT_SOLVR	(UQFILT_EXTENDMIN + 1)

/* -------------------------------------------------------------------------- */

extern int init_lib(char*);
extern void exit_lib(void);

/* -------------------------------------------------------------------------- */
/* Wallet */

#define WALLETIDLEN	SHA256_DLEN

struct wallet {
	void*	keys;
	char	id[WALLETIDLEN + 1];
	size_t	signlen;
	char*	lastsig;
};

extern int  wallet_new(struct wallet*);
extern int  wallet_load(struct wallet*, const char*);
extern int  wallet_save(const struct wallet*, const char*);
extern void wallet_free(struct wallet*);
extern int  wallet_sign(struct wallet*, const void*, size_t);

/* -------------------------------------------------------------------------- */
/* Central Authority */

extern int ca_connect(int);
extern void ca_close(int);
extern int ca_info(int);
extern int ca_curchal(int);
extern int ca_submit(int, const struct wallet*, uint64_t);
extern int ca_register(int, struct wallet*);
extern int ca_recv(int, void*, size_t);

/* -------------------------------------------------------------------------- */
/* Challenges */

#define CHAL_SORT	0	/* sorted list */
#define CHAL_RSORT	1	/* reverse sorted list */
#define CHAL_SPATH	2	/* shortest path */

struct chal {
	/* common challenge info */
	uint8_t		type;
	char		lhash[SHA256_DLEN + 1];
	uint32_t	left;
	uint64_t	id;
	uint8_t		pfix[SHA256_LEN];	/* binary (worst case) */
	size_t		plen;

	/* challenge-specific parameters */
	union {
		struct {
			size_t	nelems;
		} sl;
		struct {
			size_t	size;
			size_t	nblks;
		} sp;
	} params;
};

/*! chal_parse(info, chal) */
extern int chal_parse(const void*, size_t, struct chal*);

/* -------------------------------------------------------------------------- */
/* Challenge solvers */
/* FIXME linked-list so they can be queued properly */

struct solver {
	struct chal*	chl;		/* the challenge to solve */
	int		uq;		/* uqueue to report on */

	/* do not touch the below fields */
	void*		sl;
	void*		sp;
	uint64_t	nonce;
};

extern int  solver_init(struct solver*);
extern void solver_free(struct solver*);

extern int solver_start(struct solver*, size_t);
extern int solver_stop(int);

/* -------------------------------------------------------------------------- */
/*
 * Printing
 *
 * gprintf:	debug
 * iprintf:	info
 * wprintf:	warning
 * eprintf:	error (stderr)
 * eeprintf:	error (stderr) + errno
 *
 * The value of errno is preserved.
 */

#ifdef OIN17_DEBUG
extern void __prsrcinfo(FILE*, const char*, size_t, const char*);
#else
# define __prsrcinfo(...)
#endif
extern void __gprintf(const char*, ...);
extern void __iprintf(const char*, ...);
extern void __wprintf(const char*, ...);
extern void __eprintf(const char*, ...);
extern void __eeprintf(const char*, ...);

#define gprintf(...)	do {					\
	__prsrcinfo(stdout, __FILE__, __LINE__, __func__);	\
	__gprintf(__VA_ARGS__);					\
} while (0)

#define iprintf(...)	do {					\
	__prsrcinfo(stdout, __FILE__, __LINE__, __func__);	\
	__iprintf(__VA_ARGS__);					\
} while (0)

#define wprintf(...)	do {					\
	__prsrcinfo(stdout, __FILE__, __LINE__, __func__);	\
	__wprintf(__VA_ARGS__);					\
} while (0)

#define eprintf(...)	do {					\
	__prsrcinfo(stderr, __FILE__, __LINE__, __func__);	\
	__eprintf(__VA_ARGS__);					\
} while (0)

#define eeprintf(...)	do {					\
	__prsrcinfo(stderr, __FILE__, __LINE__, __func__);	\
	__eeprintf(__VA_ARGS__);				\
} while (0)

/* -------------------------------------------------------------------------- */

#define panic(...)	do {					\
	eprintf(__VA_ARGS__);					\
	exit(1);						\
} while (0)

/* ========================================================================== */
#endif /* !LIBCSCOINS_H */

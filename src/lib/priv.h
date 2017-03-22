/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

#ifndef LIBCSCOINS_PRIV_H
#define LIBCSCOINS_PRIV_H
/* ========================================================================== */

#include <inttypes.h>
#include <stdint.h>

#include "config.h"
#include "compat/compat.h"
#include "libcscoins.h"

/* -------------------------------------------------------------------------- */
/* Mutexes and condition variables */

typedef union mutex mutex;
typedef struct cv cv;

extern int cond_wait(cv*, mutex*);
extern int cond_signal(cv*);
extern int mutex_lock(mutex*);
extern int mutex_unlock(mutex*);

/* -------------------------------------------------------------------------- */
/* Thread pool */

/*! init_tpool() */
extern int  init_tpool(void);
/*! exit_tpool() */
extern void exit_tpool(void);

/*! tpool_start(max, func, arg)
 *
 * Execute the @func in up to @max threads.
 * @arg is passed to @func.
 *
 * Returns a task identifier. Returns <0 on error.
 */
extern int  tpool_start(size_t, void (*)(void*), void*);

/*! tpool_stop(id)
 *
 * Stop the threads assigned the given task @idé
 */
extern int  tpool_stop(int);

/* -------------------------------------------------------------------------- */
/* sha256 */

#define SHA256_LEN	32		/* (256 / 8) */
#define SHA256_DLEN	(SHA256_LEN * 2)

/*! sha256(msg, len, hash)
 *
 * Hash @len bytes of @msg and store the hash into @hash.
 *
 * WARNING @hash must be at least SHA256_LEN.
 */
extern void sha256(const void*, size_t, uint8_t*);

/* -------------------------------------------------------------------------- */
/* RSA */

/* generate a new keypair */
extern void* rsa_keygen(void);
/* save the keypair (priv,pub) */
extern int rsa_savekeys(void*, const char*);
/* load a keypair (priv,pub) */
extern void* rsa_loadkeys(const char*);
extern void* rsa_loadmemkeys(const void*, size_t);
/* sign a message with the keypair (with private key) */
extern int rsa_sign(const void*, size_t, void*, void*);
/* a signature's length */
extern size_t rsa_signlen(void*);

extern char* rsa_topem(void*);
extern uint8_t* rsa_toder(void*, size_t*);

/* -------------------------------------------------------------------------- */
/* Mersenne Twister 64 */
#define MT64_NN		312

struct mt64 {
	uint64_t	mt[MT64_NN];
	size_t		mti;
};

/*! mt64_seed(mt, seed)
 *
 * Seed the @mt MT64-PRNG with @seed.
 * If @mt is nil, the shared PRNG is used.
 *
 * WARNING The shared PRNG _MUST_ only be used by the main thread.
 */
extern void mt64_seed(struct mt64*, uint64_t);

/*! mt64_rand(mt)
 *
 * Generate a RN from the @mt PRNG. 
 * If @mt is nil, the shared PRNG is used.
 *
 * WARNING The shared PRNG _MUST_ only be used by the main thread.
 */
extern uint64_t mt64_rand(struct mt64*);

/*! mt64_randn(mt, buf, cnt)
 *
 * Generate @cnt RN from @mt and stored them in @buf.
 * If @mt is nil, the shared PRNG is used.
 *
 * WARNING The shared PRNG _MUST_ only be used by the main thread.
 */
extern void mt64_randn(struct mt64*, uint64_t*, size_t);

/* -------------------------------------------------------------------------- */
/* Generic utilities */

/*! hex2bin(src, dst)
 *
 * Convert the hexadecimal string @src and store
 * its binary version in @dst.
 *
 * WARNING @dst must be at least half the size of @src.
 *
 * Returns the number of bytes stored in @dst.
 */
extern size_t hex2bin(const char*, void*);

/* @dst must be len(@src)/2 + 1 */
extern size_t bin2hex(const void*, size_t, char*);

extern size_t u64str(uint64_t, char*);

extern void smoothsort64(uint64_t*, size_t, size_t);

extern void strrev(char*);
extern void strnrev(char*, size_t);

/* -------------------------------------------------------------------------- */
/* WebSockets
 *
 * WebSockets are abstract identifiers that can be
 * monitored with uqueue.
 *
 * They send QNOTE_CLOSED when a connection cannot be established or drops.
 * They send QNOTE_CONN when a connection is established.
 * They send QNOTE_READ when a message is available.
 */

/*! init_ws(uri)
 *
 * Initialize the WebSockets module, given the CA's @uri.
 *
 * Returns nil on success.
 */
extern int init_ws(const char*);

/*! exit_ws()
 *
 * Frees the memory allocated by the WebSockets module.
 */
extern void exit_ws(void);

/*! ws_connect(uq)
 *
 * Make a new connection to the CA. Events for the
 * associated connection will be sent via @uq.
 *
 * Returns nil on success.
 */
extern int ws_connect(int);

/*! ws_close(ws)
 *
 * Close the connection identified by @ws.
 */
extern void ws_close(int);

/*! ws_send(ws, buf, cnt)
 *
 * Send the @cnt bytes of @buf to the CA using the
 * connection identified by @ws.
 *
 * Returns the number of bytes sent to the server.
 * Returns <0 on error.
 */
extern int ws_send(int, const void*, size_t);

/*! ws_recv(ws, buf, len)
 *
 * Read a message from the CA connection @ws into
 * the buffer @buf if size @len.
 *
 * Returns the number of bytes copied into @buf
 * if AND ONLY IF @len was large enough to store
 * the whole message.
 * Returns <0 on error.
 */
extern int ws_recv(int, void*, size_t);

extern void ws_dispatch(void*);

/* -------------------------------------------------------------------------- */
/* Utilities */

/* useful shorthands */
#define REALLOC(p,n,s)	reallocarray((p), (n), (s))
#define NARRAY(a)	(sizeof(a) / sizeof((a)[0]))

/* ========================================================================== */
#endif /* !OIN17_H */

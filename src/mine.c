/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "oin17.h"

/* -------------------------------------------------------------------------- */
/*
 * Currently, the miner simply solves challenges.
 * To do so, it:
 *
 * 1) starts by querying for the current challenge
 * 2) starts solving threads for the current challenge
 * 3) waits for one (1) of two (2) events: solution or challenge
 * 4) if a new challenge arrives, go to 2
 * 5) if a solution is received, sends it and go to 3
 *
 * In the future, the miner may switch between different
 * strategies by examining the state of the game.
 */

#if 0
EXPECTED RESPONSES

    The mining module only uses the CURCHAL and SUBMIT commands. As
    such it is only expected to receive challenge infos and error
    messages (such as when transmission of a solution failed). If an
    unexpected message is received, it is printed to the screen, but,
    otherwise, ignored.

CONNECTION DROPS

    Whenever the connection with the server is severed, an event is
    sent by the WebSockets module. A new connexion is then made and
    the mining is resumed.
#endif

/* -------------------------------------------------------------------------- */

struct chl {
	struct chal	chal;
	size_t		solved;
};

/* -------------------------------------------------------------------------- */

static int uq;
static int solvpool = -1;
static struct solver solvr;

/* -------------------------------------------------------------------------- */

static void dropchal(struct chl* chl)
{
	struct uevent ue;

	if (0 > solvpool) {
		return;
	}

	solver_stop(solvpool);
	solvpool = -1;

	UQ_SET(&ue, (chl->chal.id - 1), UQFILT_SOLVR, UQ_DELETE, 0, 0, 0);
	if (0 != uevent(uq, &ue, 1, 0, 0, 0)) {
		/* we can live with dead memory for a while */
		eeprintf("Failed to drop challenge %" PRIu64, chl->chal.id);
	}
}

static int startchal(struct chl* chl)
{
	struct uevent ue;

	UQ_SET(&ue, chl->chal.id, UQFILT_SOLVR, (UQ_ADD | UQ_ENABLE), 0, 0, 0);
	if (0 != uevent(uq, &ue, 1, 0, 0, 0)) {
		if (EEXIST != errno) {
			eeprintf("Failed to register for challenge notifications");
			return -1;
		}
	}

	iprintf("#%" PRIu64 ",%hhu,%u",
	        chl->chal.id, chl->chal.type, chl->chal.left);

	solvr.chl = &chl->chal;
	solvr.uq  = uq;
	chl->solved = 0;
	solvpool = solver_start(&solvr, 1);

	return (0 > solvpool) ? -1 : 0;
}

static int handle_newchal(char* buf, size_t len, struct chl* chl)
{
	struct chal chal;

	if (0 != chal_parse(buf, len, &chal)) {
		return -1;
	}
	gprintf("New challenge received: %" PRIu64, chal.id);

	dropchal(chl);

	memset(&chl->chal, &chal, sizeof(chl->chal));

	return startchal(chl);
}

/* -------------------------------------------------------------------------- */

static int sendnonce(int sk, uint64_t non, struct wallet* wl)
{
	gprintf("Sending solution %" PRIu64, non);

	if (0 != ca_submit(sk, wl, non)) {
		eprintf("Failed to send submission");
		return -1;
	}

	return 0;
}

/* -------------------------------------------------------------------------- */

int mine(struct wallet* wl)
{
	int l;
	int sk;
	int conned;
	uint64_t n;		/* nonce */
	struct chl chl;
	struct uevent ue;
	char b[MAXMSGLEN];

	iprintf("Starting to mine");

	memset(&chl, 0, sizeof(chl));
	n = 0; /* fixes compiler warnings */

	goto L_START;

L_RESTART:
	/* destroy the current socket */
	UQ_SET(&ue, sk, UQFILT_WSOCK, UQ_DELETE, 0, 0, 0);
	if (0 != uevent(uq, &ue, 1, 0, 0, 0)) {
		wprintf("Failed to de-register websocket (ignoring)");
	}
	ca_close(sk);

L_START:
	conned = 0;
	if (0 > (sk = ca_connect(uq))) {
		eeprintf("failed to connect");
		return -1;
	}

	UQ_SET(&ue, sk, UQFILT_WSOCK, (UQ_ADD | UQ_ENABLE), 0, 0, 0);
	if (0 != uevent(uq, &ue, 1, 0, 0, 0)) {
		eeprintf("Failed to monitor websocket");
		return -1;
	}

L_WAIT:
	while (1) {
		/*
		 * conns: no need for a timer
		 * drops: no need for a timer
		 * solvr: no need for a timer
		 */
		if (1 != uevent(uq, 0, 0, &ue, 1, 0)) {
			eeprintf("Event monitoring failed");
			goto L_QUIT;
		}
		gprintf("ue=(id=%i,fi=%i,ff=%i)", ue.ident, ue.filter, ue.fflags);

		switch (ue.filter) {
		case UQFILT_WSOCK:
			if (sk != (int)ue.ident) {
				wprintf("Ignoring event on dropped socket");
				continue;
			}
			if (0 != (QNOTE_CLOSED & ue.fflags)) {
				wprintf("Connection dropped (reconnecting)");
				goto L_RESTART;
			}
			if (0 != (QNOTE_CONN & ue.fflags)) {
				conned = 1;
				/*
				 * Always send a curchal so we receive updates.
				 * Don't send pending solutions so we don't get penalized.
				 */
				iprintf("Querying chal");
				if (0 != ca_curchal(sk)) {
					eeprintf("Failed to send CURCHAL (reconnecting)");
					goto L_RESTART;
				}
				goto L_WAIT;
			}

			/* connected and waiting for response */
			if (0 > (l = ca_recv(sk, b, sizeof(b)))) {
				eeprintf("Failed to read response (reconnecting)");
				goto L_RESTART;
			}
			if (0 != strstr(b, "last_solution_hash")) {
				if (0 != handle_newchal(b, l, &chl)) {
					eprintf("Failed to start solver");
					goto L_QUIT;
				}
			} else if (0 != strstr(b, "\"error\"")) {
				wprintf("Received error message: %s", b);
				/* FIXME see cscoins/issues-13 */
				chl.solved = 0; /* assume a reset */
			} else if (0 != strstr(b, "\"submission\"")) {
				/* ignore */
			} else {
				eprintf("Cannot identify response: %s", b);
			}
			break;
		case UQFILT_SOLVR:
			if (0 != chl.solved) {
				wprintf("Ignoring late solution");
			} else {
				chl.solved = 1;
				n = (uint64_t)(ue.data);
				if (0 != conned) {
					if (0 != sendnonce(sk, n, wl)) {
						eprintf("Failed to send submission");
						/* FIXME */
					}
				} else {
					wprintf("Delaying submission: not connected");
				}
			}
			break;
		default:
			wprintf("Unexpected mining event: filter=%i", ue.filter);
			break;
		}
	}

L_QUIT:
	ca_close(sk);

	return -1;
}

/* -------------------------------------------------------------------------- */

int init_mining(void)
{
	if (0 != solver_init(&solvr)) {
		return -1;
	}

	if (0 > (uq = uqueue())) {
		return -1;
	}

	return 0;
}

void exit_mining(void)
{
	close(uq);
	solver_free(&solvr);
}

/* ========================================================================== */

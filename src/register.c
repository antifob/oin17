/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 *
 * Wallet registration
 *
 * Best-effort: if the connection fais or drops, we retry.
 * There is no point in mining if we cannot submit.
 */

/* ========================================================================== */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "oin17.h"

/* -------------------------------------------------------------------------- */

int register_wallet(struct wallet* wl)
{
	int r;
        int sk;
        int uq;
        struct uevent ue;
	char b[MAXMSGLEN];
	struct timespec ts;


        iprintf("Registering wallet: %s", wl->id);

	r = -1;

        if (0 > (uq = uqueue())) {
                eeprintf("Failed to acquire uqueue");
                return -1;
        }

	goto L_TRY;

L_RETRY:
	/* destroy the current socket */
	UQ_SET(&ue, sk, UQFILT_WSOCK, UQ_DELETE, 0, 0, 0);
	if (0 != uevent(uq, &ue, 1, 0, 0, 0)) {
		wprintf("Failed to de-register socket watch (ignoring)");
	}
	ca_close(sk);

L_TRY:
        if (0 > (sk = ca_connect(uq))) {
                eeprintf("Failed to acquire WebSocket");
                close(uq);
                return -1;
        }
        UQ_SET(&ue, sk, UQFILT_WSOCK, (UQ_ADD | UQ_ENABLE), 0, 0, 0);
        if (0 != uevent(uq, &ue, 1, 0, 0, 0)) {
		eeprintf("Failed to add WebSocket for monitoring");
		goto L_QUIT;
        }

L_WAIT1:
	/* wait for a connect event (no need for a timeout, the lib has one) */
	switch (uevent(uq, 0, 0, &ue, 1, &ts)) {
	case 1:
		if (sk != (int)ue.ident) {
			wprintf("Ignoring event on dropped socket");
			goto L_WAIT1;
		}
		if (0 != (QNOTE_CLOSED & ue.fflags)) {
			eprintf("Failed to connect to CA (retrying)");
			goto L_RETRY;
		}
		/* assuming QNOTE_CONN */
		break;
	case 0:
		eprintf("Internal error in register_wallet");
		goto L_QUIT;
	default:
		eeprintf("Failed waiting for server connection event");
		goto L_QUIT;
	}


        if (0 != ca_register(sk, wl)) {
                eprintf("Failed to send register");
		goto L_RETRY;
        }

	ts.tv_sec = 10;
	ts.tv_nsec = 0;
L_WAIT2:
        switch (uevent(uq, 0, 0, &ue, 1, &ts)) {
        case 1:
		if (sk != (int)ue.ident) {
			wprintf("Ignoring event on dropped socket");
			goto L_WAIT2;
		}
		if (0 != (QNOTE_CLOSED & ue.ident)) {
			eprintf("CA connection dropped (retrying)");
			goto L_RETRY;
		}
		/* assuming QNOTE_READ */
		if (0 > ca_recv(sk, b, sizeof(b))) {
			eeprintf("Failed to read from websocket (retrying)");
			goto L_RETRY;
		}

		if (0 != strstr(b, "wallet_id")) {
			r = 0;
		} else if (0 != strstr(b, "already_registered")) {
			r = 0;
		} else if (0 != strstr(b, "invalid signature")) {
			eprintf("Invalid signature in wallet registration");
			goto L_QUIT;
		} else {
			wprintf("Unknown registration response: %s", b);
			/* must be inspected manually */
			goto L_QUIT;
		}
		break;
        case 0: /* timer expired */
                eprintf("Failed to register wallet in time (retrying)");
		goto L_RETRY;
        default:
                eeprintf("Failed waiting for registration response (retrying)");
		/* goto L_QUIT; */
		break;
        }

L_QUIT:
        ca_close(sk);
        close(uq);

        return r;
}

/* ========================================================================== */

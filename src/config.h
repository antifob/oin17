/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 *
 * This file stores mining parameters. It SHOULD be
 * kept in sync with the cscoins/ca_config.txt.
 *
 * If the parameters change during the event, a message
 * will be displayed to the screen; indicating what should
 * be updated. If the opportunity arises, modify the below
 * values accordingly.
 *
 * TODO ^
 */

#ifndef OIN17_CONFIG_H
#define OIN17_CONFIG_H
/* ========================================================================== */

#include "lib/config.h"

#ifdef OIN17_RELEASE
# define CA_URI		"wss://cscoins.2017.csgames.org:8989/client"
#else /* !OIN17_RELEASE */
# define CA_URI		"ws://127.0.0.1:8989/client"
#endif /* !OIN17_RELEASE */

/* The team's name (empty is okay) */
#define TEAM_NAME

/* Uncomment me if the wallet was successfully registered. */
//#define WALLET_REGISTERED


#define NONCEMIN	30000000
#define NONCEMAX	CHAL_MAXNONCE

#if NONCEMIN > NONCEMAX
# error invalid nonces
#endif

/* ========================================================================== */
#endif /* !OIN17_CONFIG_H */

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

#ifndef LIBCSCOINS_CONFIG_H
#define LIBCSCOINS_CONFIG_H
/* ========================================================================== */

/* The biggest nonce we can produce + 1 */
/* see BaseChallengeGenerator.py */
#define CHAL_MAXNONCE	(99999999 + 1)

/* The expected number of numbers in the sortlist and rsortlist challenges. */
#define SL_EXPNUMS	1000

/* The algorithm used for the (r)sortlist challenges. */
/*#define SL_QUICKSORT*/
#define SL_SMOOTHSORT

/* The expected grid size in the shortpath challenge. */
#define SP_EXPSIZE	25

/* The algorithm used for the shortest path challenge. */
#define SP_DIJKSTRA

/* The maximum size of a message (register_wallet=606) */
#define MAXMSGLEN	1024


/* The number of threads reserved for:
 *
 * - solving challenges;
 * - other work (currently: libws).
 */
#define TP_RSVD_SOLVR	1
#define TP_RSVD_OTHER	1
#define TP_MINTHREADS	(TP_RSVD_SOLVR + TP_RSVD_OTHER)

/* ========================================================================== */
#endif /* !LIBCSCOINS_CONFIG_H */

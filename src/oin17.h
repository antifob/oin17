/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

#ifndef OIN17_H
#define OIN17_H
/* ========================================================================== */

#include <time.h>
#include <inttypes.h>
#include <stdint.h>

#include "lib/libcscoins.h"
#include "config.h"

/* -------------------------------------------------------------------------- */

extern int register_wallet(struct wallet*);

extern int init_mining(void);
extern void exit_mining(void);
extern int mine(struct wallet*);

/* ========================================================================== */
#endif /* !OIN17_H */

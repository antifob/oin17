/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 */

/* ========================================================================== */

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>

#include "../priv.h"

/* -------------------------------------------------------------------------- */

size_t u64str(uint64_t val, char* buf)
{
        size_t i;

        for (i = 0 ; 0 != val ; i++) {
                buf[i] = ('0' + (val % 10));
                val /= 10;
        }

	strnrev(buf, i);

        return i;
}

/* ========================================================================== */

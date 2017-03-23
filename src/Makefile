## ========================================================================== ##
##
## This file is part of oin17.
##
## Copyright 2017, Philippe Grégoire
##
## ========================================================================== ##

include ../config.mk

BIN=	oin17

SRC=	main.c \
	register.c \
	mine.c \
	\
	lib/lib.c \
        lib/wallet.c \
        \
        lib/ca/cainfo.c \
        lib/ca/close.c \
        lib/ca/connect.c \
        lib/ca/curchal.c \
        lib/ca/register.c \
        lib/ca/recv.c \
        lib/ca/submit.c \
        \
	lib/chals/getnonce.c \
	lib/chals/mkseed.c \
	lib/chals/mku64str.c \
        lib/chals/parse.c \
	lib/chals/shortpath.c \
        lib/chals/solver.c \
        lib/chals/sortlist.c \
        \
        lib/compat/reallocarray.c \
        lib/compat/strlcpy.c \
        \
        lib/crypto/mt64.c \
        lib/crypto/rsa.c \
        lib/crypto/sha256.c \
        \
        lib/utils/bin2hex.c \
	lib/utils/futex.c \
	lib/utils/heapsort64.c \
        lib/utils/hex2bin.c \
        lib/utils/printf.c \
        lib/utils/smoothsort64.c \
	lib/utils/strrev.c \
        lib/utils/tpool.c \
        lib/utils/u64str.c \
        \
        lib/ws/mbuf.c \
        lib/ws/ws.c

OBJ=	${SRC:.c=.o}

LIBS=	-lpthread -lm \
	-lwebsockets -ljsmn -luqueue -lcrypto -lssl -lrt

## -------------------------------------------------------------------------- ##

all: ${BIN}

${BIN}: ${OBJ}
	${CC} ${LDFLAGS} -o ${BIN} ${OBJ} ${LIBS}

clean:
	rm -f ${BIN} ${OBJ}

distclean: clean
	rm -f key.*

## -------------------------------------------------------------------------- ##
## dependencies

${OBJ}: config.h oin17.h key.h

key.h:
	sh ../tools/keygen.sh walletkey > key.h

## ========================================================================== ##

## ========================================================================== ##
##
## This file is part of oin17.
##
## Copyright 2017, Philippe Grégoire
##
## ========================================================================== ##
.POSIX:

PREFIX=		$${HOME}/rootfs

## -------------------------------------------------------------------------- ##

CDBG=		-g
CPPDBG=		-DOIN17_DEBUG=1
#CPPREL=	-DOIN17_RELEASE

CC=		gcc

CFLAGS=		-pedantic -std=c99 -Wall -Wextra -Werror -O2 ${CDBG}

# Do not touch _XOPEN_SOURCE
CPPFLAGS=	-I${PREFIX}/include -D_XOPEN_SOURCE=700 ${CPPDBG} ${CPPREL}

LDFLAGS=	-Wl,-z,noexecstack -Wl,-z,now -Wl,-z,relro \
		-L${PREFIX}/lib -Wl,-rpath,${PREFIX}/lib ${CDBG}

## -------------------------------------------------------------------------- ##

.SUFFIXES: .c .o
.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} -c -o $@ $<

## ========================================================================== ##

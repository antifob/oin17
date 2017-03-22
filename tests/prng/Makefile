## ========================================================================== ##
##
## This file is part of oin17.
##
## Copyright 2017, Philippe Grégoire
##
## ========================================================================== ##
## TODO regenerate mt64.c using current project's file

# 17 first digits of pi
SEED=		31415926535897932

NTESTS=		256
OUTA=		A
OUTB=		B
OUTS=		${OUTA} ${OUTB}

## -------------------------------------------------------------------------- ##

all: mt64-a mt64-b

mt64-a: mt19937-64.c
	${CC} -DNTESTS=${NTESTS} -o $@ mt19937-64.c

mt64-b: mt64.c
	${CC} -DNTESTS=${NTESTS} -o $@ mt64.c

clean:
	rm -f mt64-a mt64-b ${OUTS}

test: all
	./mt64-a ${SEED} > ${OUTA}
	./mt64-b ${SEED} > ${OUTB}
	diff ${OUTA} ${OUTB}

## ========================================================================== ##

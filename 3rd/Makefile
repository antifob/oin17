## ========================================================================== ##
##
## This file is part of oin17.
##
## Copyright 2017, Philippe Grégoire
##
## ========================================================================== ##

include ../config.mk

LIBS=	libws jsmn uqueue

## -------------------------------------------------------------------------- ##

all:
	for lib in ${LIBS}; do ${MAKE} $$lib; done

install:
	for lib in ${LIBS}; do ${MAKE} $$lib-install; done

clean: distclean

distclean:
	for lib in ${LIBS}; do ${MAKE} $$lib-distclean; done

## -------------------------------------------------------------------------- ##

include mk/libws.mk
include mk/jsmn.mk
#include mk/bmake.mk
include mk/uqueue.mk

## ========================================================================== ##

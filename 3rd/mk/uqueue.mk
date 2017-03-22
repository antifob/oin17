## ========================================================================== ##
##
## This file is part of oin17.
##
## Copyright 2017, Philippe Grégoire
##
## ========================================================================== ##

UQ_VERS=	master-157e5c6abf246d117fb2d832805b069c1dafdcd0

uqueue:
	gzip -cd dist/uqueue-${UQ_VERS}.tar.gz | tar x -f-
	(cd uqueue-${UQ_VERS}; \
		patch -Np1 < ../patches/uqueue.diff; \
		sh ./configure --prefix=${PREFIX}; \
		cd src; bmake; )

uqueue-install: uqueue
	(cd uqueue-${UQ_VERS}/src; \
		bmake install; )

uqueue-distclean:
	rm -rf uqueue-${UQ_VERS}

## ========================================================================== ##

## ========================================================================== ##
##
## This file is part of oin17.
##
## Copyright 2017, Philippe Grégoire
##
## ========================================================================== ##

# version 2.2.0 is broken
LIBWS_VERS=	2.2.0
#2.1.1
LIBWS_OPTS=\
	-DCMAKE_BUILD_TYPE=Release \
	-DLWS_WITHOUT_SERVER=ON \
	-DLWS_WITHOUT_TESTAPPS=ON

libws:
	gzip -cd dist/libwebsockets-${LIBWS_VERS}.tar.gz | tar x -f-
	(cd libwebsockets-${LIBWS_VERS}; \
		if [ -f ../patches/libws-${LIBWS_VERS}.diff ]; then \
			patch -Np1 < ../patches/libws-${LIBWS_VERS}.diff; \
		fi; \
		cmake \
			${LIBWS_OPTS} \
			-DCMAKE_INSTALL_PREFIX=${PREFIX} \
			.; \
		${MAKE} VERBOSE=1; )

libws-install: libws
	(cd libwebsockets-${LIBWS_VERS}; \
		${MAKE} VERBOSE=1 install; )

libws-distclean:
	rm -rf libwebsockets-${LIBWS_VERS}

## ========================================================================== ##

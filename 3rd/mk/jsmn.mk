## ========================================================================== ##
##
## This file is part of oin17.
##
## Copyright 2017, Philippe Grégoire
##
## ========================================================================== ##

jsmn:
	gzip -cd dist/jsmn.tar.gz | tar x -f-
	(cd jsmn; \
		${CC} ${CPPFLAGS} -DJSON_STRICT=1 ${CFLAGS} -c -o jsmn.o jsmn.c; \
		${AR} cr libjsmn.a jsmn.o; )

jsmn-install: jsmn
	(cd jsmn; \
		mkdir -p "${PREFIX}/lib" "${PREFIX}/include"; \
		cp libjsmn.a "${PREFIX}/lib"; \
		cp jsmn.h "${PREFIX}/include"; )

jsmn-distclean:
	rm -rf jsmn

## ========================================================================== ##

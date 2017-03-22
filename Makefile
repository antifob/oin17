## ========================================================================== ##
##
## This file is part of oin17.
##
## Copyright 2017, Philippe Grégoire
##
## ========================================================================== ##

include config.mk

SUBDIRS=	3rd src

## -------------------------------------------------------------------------- ##

all: oin17

oin17:
	(cd src; ${MAKE};)

deps:
	(cd 3rd; ${MAKE} install;)

clean:
	@for dir in ${SUBDIRS}; do (cd $$dir; ${MAKE} clean;); done

images: docker

## -------------------------------------------------------------------------- ##

DOCKER_NAME=		oin17

docker:
	docker pull debian
	docker build -t ${DOCKER_NAME}

## -------------------------------------------------------------------------- ##

run:
	docker run -t ${DOCKER_NAME}

## ========================================================================== ##

## ========================================================================== ##
##
## This file is part of oin17.
##
## Copyright 2017, Philippe Grégoire
##
## Generate a private RSA key.
##
## ========================================================================== ##
set -e

PROGNAME=`basename -- "${0}"`

if [ 0 -eq $# ]; then
	printf 'usage: %s <out>\n' "${PROGNAME}" >&2
	exit 1
fi

openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:1024 2>/dev/null

## ========================================================================== ##
exit $?

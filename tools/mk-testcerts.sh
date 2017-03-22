## -------------------------------------------------------------------------- ##
##
## This file is part of oin17.
##
## Copyright 2017, Philippe Grégoire
##
## ========================================================================== ##
set -e

PROGBASE=`dirname -- "${0}"`
PROGBASE=`cd "${PROGBASE}"; pwd`

CADIR=./testcerts

## -------------------------------------------------------------------------- ##
clear

cat << __EOF
You will be asked 3 names. Enter:

CA
tralala
cscoins.2017.csgames.org

To reset, remove the folder ${CADIR} and run this script again.

Continue? [y/N]
__EOF

read yn
case ${yn} in
y|Y)	;;
*)	exit 1;;
esac

## -------------------------------------------------------------------------- ##

cd "${PROGBASE}"
mkdir "${CADIR}"
mkdir "${CADIR}/crt"
touch "${CADIR}/index.txt"
touch "${CADIR}/index.txt.attr"
printf '01\n' > "${CADIR}/serial.txt"

## -------------------------------------------------------------------------- ##

# ca key
openssl genpkey \
	-algorithm RSA \
	-out "${CADIR}/ca.key"

# ca cert
openssl req \
	-new \
	-x509 \
	-days 10 \
	-key "${CADIR}/ca.key" \
	-out "${CADIR}/ca.crt" \
	-config ./openssl.conf

mksrvkey() {
	# server key
	openssl genpkey \
		-algorithm RSA \
		-out "${CADIR}/srv${1}.key"

	# server csr
	openssl req \
		-new \
		-key "${CADIR}/srv${1}.key" \
		-out ./testcerts/srv${1}.csr \
		-config ./openssl.conf

	# sign
	openssl ca \
		-config ./openssl.conf \
		-out "${CADIR}/srv${1}.crt" \
		-infiles "${CADIR}/srv${1}.csr"
}

mksrvkey 1
mksrvkey 2

## -------------------------------------------------------------------------- ##
clear

cat << __EOF
Congrats! Keys were placed into ${CADIR}.

test #1
Configure the CSCoins CA to use srv1.*.
Ensure it is NOT accepted.

test #2
Configure the CSCoins CA to use srv2.*.
Ensure it is NOT accepted.

preparation
Install the root certificate system-wide.

test #3
Configure the CSCoins CA to use srv1.*.
Ensure it is NOT accepted.

test #4
Configure the CSCoins CA to use srv2.*.
Ensure it is ACCEPTED.

Do not forget to add cscoins.2017.csgames.org to /etc/hosts.
__EOF

## ========================================================================== ##
exit 0

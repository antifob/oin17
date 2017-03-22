## ========================================================================== ##
##
## This file is part of ion17.
##
## Copyright 2017, Philippe Grégoire
##
## ========================================================================== ##
set -e

PROGNAME=`basename -- "${0}"`

REPO=https://github.com/csgames/cscoins
CADB=cacoins
CAUSER=cacoins
CAPASS=

## -------------------------------------------------------------------------- ##

usage() {
	printf 'usage: %s [-hv] <user>\n' "$PROGNAME"
}

while getopts hv arg; do
	case $arg in
	h) usage; exit 0;;
	v) O_VERB=1;;
	*) usage >&2; exit 1;;
	esac
done
shift `expr ${OPTIND} \- 1`

if [ 0 -eq $# ]; then
	usage >&2
	exit 1
fi

## -------------------------------------------------------------------------- ##

if [ 0 -ne `id -u` ]; then
	printf '[E] This program needs to be run with root privileges\n' >&2
	exit 1
fi

## -------------------------------------------------------------------------- ##

printf '[I] Creating dedicated user account.\n'
if ! grep "^${1}:" /etc/passwd >/dev/null; then
	useradd -s /bin/sh -m ${1}
fi

printf '[I] Installing packages.\n'
if [ -x /usr/bin/apt-get ]; then
	for pkg in python3-pip python3-setuptools mysql-server \
	           mysql-utilities gcc libmysqlclient-dev libpython3-dev \
	           python3-crypto python3-wheel python3-mysqldb
	do
		apt-get install -y ${pkg}
	done
else
	printf '[E] Unknown package manager. Update this program.\n' >&2
	exit 1
fi

cat > "/home/${1}/install-ca" << __EOF
set -e

cd /home/$1

printf '[I] Cloning source repository.\n'
if [ ! -d "`basename ${REPO}`" ]; then
	git clone "${REPO}"
fi

printf '[I] Installing pip packages.\n'
for pip in websockets asyncio mysqlclient; do
	pip3 install \${pip}
done

printf '[I] Creating MySQL database.\n'
__dbcmd() { printf '%s\n' "\$1" | mysql -u root; }
__dbcmd 'create database ${CADB};'
__dbcmd 'create user ${CAUSER}@localhost;'
__dbcmd "grant usage on *.* to ${CAUSER}@localhost identified by '${CAPASS}';"
__dbcmd 'grant all privileges on ${CADB}.* to ${CAUSER}@localhost;'

exit 0
__EOF

su -c "/bin/sh -x /home/${1}/install-ca"

## ========================================================================== ##
exit 0

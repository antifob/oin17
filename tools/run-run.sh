## ========================================================================== ##
##
## This file is part of oin17.
##
## Copyright 2017, Philippe Grégoire
##
## ========================================================================== ##
set -e

D=`dirname -- "$0"`
B=`cd "${D}/.."; pwd`

cd "${B}/src"

while :; do
	./oin17
done

## ========================================================================== ##

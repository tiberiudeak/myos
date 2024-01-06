#!/bin/bash

set -e

. ./headers.sh

for PROJECT in boot; do
	(cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install)
done

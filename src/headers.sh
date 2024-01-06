#!/bin/bash

set -e
. ./config.sh

mkdir -p "$SYSROOT"

for PROJECT in boot; do
	(cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install-headers)
done

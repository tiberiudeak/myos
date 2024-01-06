#!/bin/bash

set -e
. ./config.sh

for PROJECT in boot; do
	(cd $PROJECT && $MAKE clean)
done

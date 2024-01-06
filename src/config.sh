SYSTEM_HEADER_PROJECTS="boot libc kernel"
PROJECTS="boot libc kernel"

export MAKE=${MAKE:-make}
export HOST=${HOST:-$(./default-host.sh)}

export AR=${HOST}-ar
export AS=${HOST}-as
export CC=${HOST}-gcc
export LD=${HOST}-ld

export PREFIX=/usr
export EXEC_PREFIX=$PREFIX
export BOOTDIR=/boot
export LIBDIR=$EXEC_PREFIX/lib
export INCLUDEDIR=$PREFIX/include

export CFLAGS='-O2 -g'
export CPPFLAGS=''

export SYSROOT="$(pwd)/sysroot"
export CC="$CC --sysroot=$SYSROOT"

if echo "$HOST" | grep -Eq -- '-elf($|-)'; then
	export CC="$CC -isystem=$INCLUDEDIR"
fi

# print all variables
echo "MAKE: $MAKE"
echo "HOST: $HOST"
echo "AR: $AR"
echo "AS: $AS"
echo "CC: $CC"
echo "PREFIX: $PREFIX"
echo "EXEC_PREFIX: $EXEC_PREFIX"
echo "BOOTDIR: $BOOTDIR"
echo "LIBDIR: $LIBDIR"
echo "INCLUDEDIR: $INCLUDEDIR"
echo "CFLAGS: $CFLAGS"
echo "CPPFLAGS: $CPPFLAGS"
echo "SYSROOT: $SYSROOT"
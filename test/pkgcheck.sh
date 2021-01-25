#!/bin/sh

usage() {
  cat <<"_EOF_"
Usage: sh test/pkgcheck.sh [--zlib-compat]

Verifies that the various build systems produce identical results on a Unixlike system.
If --zlib-compat, tests with zlib compatible builds.

To build the 32 bit version for the current 64 bit arch:

$ sudo apt install ninja-build diffoscope gcc-multilib
$ export CMAKE_ARGS="-DCMAKE_C_FLAGS=-m32" CFLAGS=-m32 LDFLAGS=-m32
$ sh test/pkgcheck.sh

To cross-build, install the appropriate qemu and gcc packages,
and set the environment variables used by configure or cmake.
On Ubuntu, for example (values taken from .github/workflows/pkgconf.yml):

arm HF:
$ sudo apt install ninja-build diffoscope qemu gcc-arm-linux-gnueabihf libc6-dev-armhf-cross
$ export CHOST=arm-linux-gnueabihf
$ export CMAKE_ARGS="-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm.cmake -DCMAKE_C_COMPILER_TARGET=${CHOST}"

aarch64:
$ sudo apt install ninja-build diffoscope qemu gcc-aarch64-linux-gnu libc6-dev-arm64-cross
$ export CHOST=aarch64-linux-gnu
$ export CMAKE_ARGS="-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-aarch64.cmake -DCMAKE_C_COMPILER_TARGET=${CHOST}"

ppc (32 bit big endian):
$ sudo apt install ninja-build diffoscope qemu gcc-powerpc-linux-gnu libc6-dev-powerpc-cross
$ export CHOST=powerpc-linux-gnu
$ export CMAKE_ARGS="-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-powerpc.cmake"

ppc64le:
$ sudo apt install ninja-build diffoscope qemu gcc-powerpc64le-linux-gnu libc6-dev-ppc64el-cross
$ export CHOST=powerpc64le-linux-gnu
$ export CMAKE_ARGS="-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-powerpc64le.cmake"

then:
$ export CC=${CHOST}-gcc
$ sh test/pkgcheck.sh [--zlib-compat]

Note: on Mac, you may also need to do 'sudo xcode-select -r' to get cmake to match configure/make's behavior (i.e. omit -isysroot).
_EOF_
}

set -ex

# Caller can also set CMAKE_ARGS or CONFIGURE_ARGS if desired
CMAKE_ARGS=${CMAKE_ARGS}
CONFIGURE_ARGS=${CONFIGURE_ARGS}

case "$1" in
--zlib-compat)
  suffix=""
  CMAKE_ARGS="$CMAKE_ARGS -DZLIB_COMPAT=ON"
  CONFIGURE_ARGS="$CONFIGURE_ARGS --zlib-compat"
  ;;
"")
  suffix="-ng"
  ;;
*)
  echo "Unknown arg '$1'"
  usage
  exit 1
  ;;
esac

if ! test -f "configure"
then
  echo "Please run from top of source tree"
  exit 1
fi

# Tell GNU's ld etc. to use Jan 1 1970 when embedding timestamps
# Probably only needed on older systems (ubuntu 14.04, BSD?)
export SOURCE_DATE_EPOCH=0
case $(uname) in
Darwin)
  # Tell Apple's ar etc. to use zero timestamps
  export ZERO_AR_DATE=1
  # What CPU are we running on, exactly?
  sysctl -n machdep.cpu.brand_string
  sysctl -n machdep.cpu.features
  sysctl -n machdep.cpu.leaf7_features
  sysctl -n machdep.cpu.extfeatures
  ;;
esac

# Use same compiler for make and cmake builds
if test "$CC"x = ""x
then
  if clang --version
  then
    export CC=clang
  elif gcc --version
  then
    export CC=gcc
  fi
fi
export LD=ld
# Use the same version of macOS SDK when linking
case $(uname) in
Darwin)
  sysroot=$(xcrun --show-sdk-path)
  export CFLAGS="$CFLAGS -mmacosx-version-min=10.15"
  export LDFLAGS="$LDFLAGS -isysroot $sysroot"
  export MACOSX_DEPLOYMENT_TARGET=10.15
  ;;
esac
# New build system
# Happens to delete top-level zconf.h
# (which itself is a bug, https://github.com/madler/zlib/issues/162 )
# which triggers another bug later in configure,
# https://github.com/madler/zlib/issues/499
#v -DCMAKE_C_CREATE_SHARED_LIBRARY="/usr/bin/xcrun libtool -D -o <TARGET> <LINK_FLAGS> <OBJECTS>"
rm -rf btmp2 pkgtmp2
mkdir btmp2 pkgtmp2
export LD=ld64
export DESTDIR=$(pwd)/pkgtmp2
cd btmp2
  cmake -G Ninja ${CMAKE_ARGS} .. -DCMAKE_OSX_SYSROOT=$sysroot -DCMAKE_SKIP_RPATH=ON
  ninja -v
  ninja install
cd ..
echo ZERO_AR_DATE = $ZERO_AR_DATE
# Original build system
rm -rf btmp1 pkgtmp1
mkdir btmp1 pkgtmp1
export DESTDIR=$(pwd)/pkgtmp1
cd btmp1
  case $(uname) in
  Darwin)
    export LDFLAGS="$LDFLAGS -Wl,-headerpad_max_install_names"
    ;;
  esac
  ../configure $CONFIGURE_ARGS
  make
  make install
cd ..

repack_ar() {
  if ! cmp --silent pkgtmp1/usr/local/lib/libz$suffix.a pkgtmp2/usr/local/lib/libz$suffix.a
  then
    echo "libz$suffix.a does not match.  Probably filenames differ (.o vs .c.o).  Unpacking and renaming..."
    # Note: %% is posix shell syntax meaning "Remove Largest Suffix Pattern", see
    # https://pubs.opengroup.org/onlinepubs/009695399/utilities/xcu_chap02.html#tag_02_06_02
    cd pkgtmp1; ar x usr/local/lib/libz$suffix.a; rm usr/local/lib/libz$suffix.a; cd ..
    cd pkgtmp2; ar x usr/local/lib/libz$suffix.a; rm usr/local/lib/libz$suffix.a; for a in *.c.o; do mv $a ${a%%.c.o}.o; done; cd ..
    # Also, remove __.SYMDEF SORTED if present, as it has those funky .c.o names embedded in it.
    rm -f pkgtmp[12]/__.SYMDEF\ SORTED
  fi
}

case $(uname) in
Darwin)
  # Remove the build uuid.
  dylib1=$(find pkgtmp1 -type f -name '*.dylib*')
  dylib2=$(find pkgtmp2 -type f -name '*.dylib*')
  strip -x -D -no_uuid "$dylib1"
  strip -x -D -no_uuid "$dylib2"
  ;;
esac

# The ar on newer systems defaults to -D (i.e. deterministic),
# but FreeBSD 12.1, Debian 8, and Ubuntu 14.04 seem to not do that.
# I had trouble passing -D safely to the ar inside CMakeLists.txt,
# so punt and unpack the archive if needed before comparing.
# Also, cmake uses different .o suffix anyway...
repack_ar

if diff -Nur pkgtmp1 pkgtmp2
then
  echo pkgcheck-cmake-bits-identical PASS
else
  echo pkgcheck-cmake-bits-identical FAIL
  dylib1=$(find pkgtmp1 -type f -name '*.dylib*' -print -o -type f -name '*.so.*' -print)
  dylib2=$(find pkgtmp2 -type f -name '*.dylib*' -print -o -type f -name '*.so.*' -print)
  diffoscope $dylib1 $dylib2 | cat
  exit 1
fi

rm -rf btmp1 btmp2 pkgtmp1 pkgtmp2

# any failure would have caused an early exit already
echo "pkgcheck: PASS"

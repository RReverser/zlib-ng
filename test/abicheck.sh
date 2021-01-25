#!/bin/sh
set -ex
TESTDIR="$(cd $(dirname "$0"); pwd)"

usage() {
    cat <<_EOF_
Usage: $0 [--zlib-compat][--refresh][--refresh-if]

Build shared library with -ggdb, then compare its ABI to the stable
ABI, and abort if differences found.

Options:
--zlib-compat  - check the ABI of the zlib-compatible flavor of zlib-ng.
--refresh      - build the reference library and extract its ABI rather than using a stored ABI file.
--refresh-if   - refresh only if ABI file not present.

Obeys CHOST, CONFIGURE_ARGS, CFLAGS, and LDFLAGS.

Requires libabigail (on Ubuntu, install package abigail-tools).
_EOF_
}

# Print the multiarch tuple for the current (non-cross) machine,
# or the empty string if unavailable.
detect_chost() {
    dpkg-architecture -qDEB_HOST_MULTIARCH ||
     $CC -print-multiarch ||
     $CC -print-search-dirs | sed 's/:/\n/g' | grep -E '^/lib/[^/]+$' | sed 's%.*/%%' ||
     true
}

if ! test -f "configure"
then
  echo "Please run from top of source tree"
  exit 1
fi

suffix="-ng"
CONFIGURE_ARGS_NG="$CONFIGURE_ARGS"
refresh=false
refresh_if=false
for arg
do
  case "$arg" in
  --zlib-compat)
    suffix=""
    CONFIGURE_ARGS_NG="$CONFIGURE_ARGS_NG --zlib-compat"
    ;;
  --refresh)
    refresh=true
    ;;
  --refresh_if)
    refresh_if=true
    ;;
  --help)
    usage
    exit 0
    ;;
  *)
    echo "Unknown arg '$arg'"
    usage
    exit 1
    ;;
  esac
done

# Choose reference repo and commit
if test "$suffix" = ""
then
  # Reference is zlib 1.2.11
  ABI_GIT_REPO=https://github.com/madler/zlib.git
  ABI_GIT_COMMIT=v1.2.11
else
  # Reference should be the tag for zlib-ng 2.0
  # but until that bright, shining day, use some
  # random recent SHA.  Annoyingly, can't shorten it.
  ABI_GIT_REPO=https://github.com/zlib-ng/zlib-ng.git
  ABI_GIT_COMMIT=56ce27343bf295ae9457f8e3d38ec96d2f949a1c
fi
# FIXME: even when using a tag, check the hash.


ABIFILE="test/abi/zlib$suffix-$ABI_GIT_COMMIT-$CHOST$M32.abi"
if ! $refresh && $refresh_if && ! test -f "$ABIFILE"
then
  refresh=true
fi
abidw --version

if $refresh
then
  # Check out reference source
  rm -rf btmp1
  mkdir -p btmp1/src.d
  cd btmp1/src.d
  git init
  git remote add origin $ABI_GIT_REPO
  git fetch origin $ABI_GIT_COMMIT
  git reset --hard FETCH_HEAD
  cd ..
  uname -a
  # Build unstripped, uninstalled, very debug shared library
  CFLAGS="$CFLAGS -ggdb" sh src.d/configure $CONFIGURE_ARGS
  make -j2
  cd ..
  # Find shared library, extract its abi
  dylib1=$(find btmp1 -type f -name '*.dylib*' -print -o -type f -name '*.so.*' -print)
  abidw $dylib1 > "$ABIFILE"
  # Maintainers may wish to check $ABIFILE into git when a new
  # target is added, or when a major release happens that is
  # intended to change the ABI.  Alternately, this script could
  # just always rebuild the reference source, and dispense with
  # caching abi files in git (but that would slow builds down).
fi

if test -f "$ABIFILE"
then
  ABIFILE="$ABIFILE"
else
  echo "abicheck: SKIP: $ABIFILE not found; rerun with --refresh or --refresh_if"
  exit 0
fi
echo uname -a
uname -a
# Build unstripped, uninstalled, very debug shared library
rm -rf btmp2
mkdir btmp2
cd btmp2
CFLAGS="$CFLAGS -ggdb" ../configure $CONFIGURE_ARGS_NG
make -j2
cd ..
# Find shared library, extract its abi
dylib2=$(find btmp2 -type f -name '*.dylib*' -print -o -type f -name '*.so.*' -print)
abidw $dylib2 > btmp2/zlib${suffix}-built.abi
abidiff --version
gcc --version
echo "XYZ"
cat btmp1/configure.log
echo "ABC"
cat btmp2/configure.log
uname -a
# Compare it to the reference
# FIXME: use --no-added-syms for now, but we probably want to be more strict.
if abidiff --no-added-syms --suppressions test/abi/ignore "$ABIFILE" btmp2/zlib${suffix}-built.abi
then
  echo "abicheck: PASS"
else
  echo "abicheck: FAIL"
  exit 1
fi

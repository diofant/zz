#!/bin/sh

set -e -x

GMP_VERSION=6.3.0
GMP_DIR=gmp-${GMP_VERSION}
GMP_URL=https://ftp.gnu.org/gnu/gmp/${GMP_DIR}.tar.xz
PREFIX="$(pwd)/.local/"
CFLAGS=

curl --retry 3 --silent --remote-name ${GMP_URL}
tar --extract --file ${GMP_DIR}.tar.xz
cd ${GMP_DIR}

for f in ../scripts/*.diff
do
  patch --strip 1 < $f
done

# config.guess uses microarchitecture and configfsf.guess doesn't
# We replace config.guess with configfsf.guess to avoid microarchitecture
# specific code in common code.
rm config.guess && mv configfsf.guess config.guess && chmod +x config.guess

./configure --enable-fat \
            --enable-shared \
            --disable-static \
            --with-pic \
            --disable-alloca \
            --quiet

sudo make --silent all install

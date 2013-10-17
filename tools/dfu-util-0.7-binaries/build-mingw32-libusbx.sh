#!/bin/sh

#
# The dfu-util binaries for Windows (win32) were
# built on Ubuntu 12.04 using this build script.
#
# To install needed build dependencies run:
#  sudo apt-get build-dep libusb-1.0-0 dfu-util
#  sudo apt-get install mingw32
#

set -e

BUILD_DIR=/tmp/mingw
MINGW_VERSION=i586-mingw32msvc

[ -d $BUILD_DIR ] || mkdir -p $BUILD_DIR
cd $BUILD_DIR

# get libusb sources
[ -d libusbx-1.0.14 ] || { wget -O libusb-1.0.14.tar.bz2 http://sourceforge.net/projects/libusbx/files/releases/1.0.14/source/libusbx-1.0.14.tar.bz2/download; tar jxvf libusb-1.0.14.tar.bz2 ;}
cd libusbx-1.0.14
wget https://github.com/libusbx/libusbx/commit/09759d5836766fb3b886824cd669bc0dfc149d00.patch
patch -p1 < 09759d5836766fb3b886824cd669bc0dfc149d00.patch
PKG_CONFIG_PATH=$BUILD_DIR/lib/pkgconfig ./configure --host=$MINGW_VERSION --prefix=$BUILD_DIR
make
make install
cd ..

# get dfu-util sources
[ -d dfu-util-0.7 ] || { wget http://dfu-util.gnumonks.org/releases/dfu-util-0.7.tar.gz ; tar zxvf dfu-util-0.7.tar.gz ;}
cd dfu-util-0.7
PKG_CONFIG_PATH=$BUILD_DIR/lib/pkgconfig ./configure --host=$MINGW_VERSION --prefix=$BUILD_DIR
make
make install
cd ..

#!/bin/sh

#FIXME
#YOUR_LOCAL_PATH=XXX
TOOLCHAIN_PATH=$YOUR_LOCAL_PATH/friendlywrt23-rk3328/friendlywrt/staging_dir/toolchain-aarch64_generic_gcc-12.3.0_musl/bin

export PATH=$TOOLCHAIN_PATH:$PATH
export STAGING_DIR=$TOOLCHAIN_PATH/..

export CC=aarch64-openwrt-linux-musl-gcc
export AR=aarch64-openwrt-linux-musl-ar
export RANLIB=aarch64-openwrt-linux-musl-ranlib
export STRIP=aarch64-openwrt-linux-musl-strip

make clean
make

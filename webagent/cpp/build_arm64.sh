#!/bin/sh

# Copyright (c) 2024-2025 Chunghan Yi <chunghan.yi@gmail.com>
# SPDX-License-Identifier: MIT

#YOUR_LOCAL_PATH=XXX
TOOLCHAIN_PATH=$YOUR_LOCAL_PATH/friendlywrt21-rk3399/friendlywrt/staging_dir/toolchain-aarch64_generic_gcc-11.2.0_musl/bin

export PATH=$TOOLCHAIN_PATH:$PATH
export STAGING_DIR=$TOOLCHAIN_PATH/..

export CC=aarch64-openwrt-linux-musl-gcc
export CPP=aarch64-openwrt-linux-musl-g++
export AR=aarch64-openwrt-linux-musl-ar
export RANLIB=aarch64-openwrt-linux-musl-ranlib

CPPPATH=$(pwd)
if [ ! -d ./external/spdlog ]; then
	cd external
	git clone https://github.com/gabime/spdlog
	cd spdlog
	mkdir build && cd build

#	cmake .. && cmake --build .
	cmake -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=arm64v8 \
		-DCMAKE_CXX_COMPILER=$TOOLCHAIN_PATH/aarch64-openwrt-linux-musl-g++ \
		..
	make

	if [ ! -d $CPPPATH/external/lib ]; then
		mkdir $CPPPATH/external/lib > /dev/null 2>&1
	fi
	cp ./libspdlog.a ../../lib > /dev/null 2>&1
	cd ..
	cp -R ./include ../lib > /dev/null 2>&1
	cd $CPPPATH
fi

if [ ! -d ./build ]; then
	mkdir -p build
fi
cd build
cmake .. && make

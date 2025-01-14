#!/bin/sh

# Copyright (c) 2024-2025 Chunghan Yi <chunghan.yi@gmail.com>
# SPDX-License-Identifier: MIT

CPPPATH=$(pwd)
if [ ! -d ./external/spdlog ]; then
	cd external
	git clone https://github.com/gabime/spdlog
	cd spdlog
	mkdir build && cd build
	cmake .. && cmake --build .

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

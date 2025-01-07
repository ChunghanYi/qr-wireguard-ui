#!/bin/bash -e
# vim: set ts=4:
#
# This script creates a quantum-resistant wireguard package for nanopi openwrt.
# (This shell script has been tested in the Ubuntu 22.04 LTS environment)
#
# Copyright (c) 2024 Chunghan Yi <chunghan.yi@gmail.com>
# SPDX-License-Identifier: Apache-2.0
#
# ///////////////////////////////////////////////////////////////////////////////

VERSION=0.9.00
CWD=$(pwd)
OUTPUT=$CWD/output
INST_PKG_PATH=$CWD/rootfs/qrwg
LIB_PATH=$CWD/rootfs/lib/nanopi

#FIXME
#YOUR_LOCAL_PATH=XXX
#See https://wiki.friendlyelec.com/wiki/index.php/NanoPi_R2S_Plus
TOOLCHAIN_PATH=$YOUR_LOCAL_PATH/friendlywrt23-rk3328/friendlywrt/staging_dir/toolchain-aarch64_generic_gcc-12.3.0_musl/bin

export PATH=$TOOLCHAIN_PATH:$PATH
export STAGING_DIR=$TOOLCHAIN_PATH/..

# ///////////////////////////////////////////////////////////////////////////////

print_green()
{
	if tty -s 2>/dev/null; then
		echo -ne "\033[32m"
		echo -n "$@"
		echo -e "\033[0m"
	else
		echo "$@"
	fi
}

print_red()
{
	if tty -s 2>/dev/null; then
		echo -ne "\033[31m"
		echo -n "$@"
		echo -e "\033[0m"
	else
		echo "$@"
	fi
}

#
# https://github.com/ngoduykhanh/wireguard-ui
#
build_wireguard_ui()
{
	cd $CWD

	cd webui/wireguard-ui
	if [ $1 = "release" ]; then
		if [ ! -d ./node_modules ]; then
			./prepare_assets.sh
			go get github.com/ChunghanYi/qr-wireguard-ui/webui/beplugin
		fi
		GOOS=linux GOARCH=arm64 go build -o wireguard-ui	
		cp ./wireguard-ui $INST_PKG_PATH/usr/bin/qrwg
		chmod 755 $INST_PKG_PATH/usr/bin/qrwg/wireguard-ui
	elif [ $1 = "clean" ]; then
		GOOS=linux GOARCH=arm64 go clean
	fi

	cd $CWD
}

#
# Backend agent for wireguard-ui
#
build_web_agent()
{
	cd $CWD

	cd webagent
	if [ $1 = "release" ]; then
		make -f Makefile.arm64 clean
		make -f Makefile.arm64
		cp ./web-agentd $INST_PKG_PATH/usr/bin/qrwg
		chmod 755 $INST_PKG_PATH/usr/bin/qrwg/web-agentd
	elif [ $1 = "clean" ]; then
		make -f Makefile.arm64 clean
	fi

	cd $CWD
}

#
# Wireguard shell
#
build_vtysh()
{
	cd $CWD
	if [ $1 = "release" ]; then
		cd wgshell/external

		if [ ! -d ./ncurses-6.2 ]; then
			wget https://ftp.gnu.org/gnu/ncurses/ncurses-6.2.tar.gz
			tar xzf ncurses-6.2.tar.gz
		fi
		cd ncurses-6.2
		export CC=aarch64-openwrt-linux-musl-gcc
		export AR=aarch64-openwrt-linux-musl-ar
		export RANLIB=aarch64-openwrt-linux-musl-ranlib
		./configure --host=aarch64-openwrt-linux-musl --with-shared
		make clean; make
		cp -r lib/libncurses.so* $LIB_PATH
		cd ..

		if [ ! -d ./readline-8.1 ]; then
			wget https://ftp.gnu.org/gnu/readline/readline-8.1.tar.gz
			tar -xzf readline-8.1.tar.gz
		fi
		cd readline-8.1
		rm -rf ./output > /dev/null 2>&1
		mkdir -p output > /dev/null 2>&1
		export CC=aarch64-openwrt-linux-musl-gcc
		export AR=aarch64-openwrt-linux-musl-ar
		export RANLIB=aarch64-openwrt-linux-musl-ranlib
		CPATH=$(pwd)
		./configure --host=aarch64-openwrt-linux-musl --prefix=$CPATH/output
		make clean; make
		make install > /dev/null 2>&1
		cp -r output/lib/libreadline.so* $LIB_PATH

		cd ../../vtysh
		make -f ./Makefile clean
		CC=aarch64-openwrt-linux-musl-gcc AR=aarch64-openwrt-linux-musl-ar make -f ./Makefile
		aarch64-openwrt-linux-musl-strip ./vtysh
		cp ./vtysh $INST_PKG_PATH/usr/bin/qrwg
		chmod 755 $INST_PKG_PATH/usr/bin/qrwg/vtysh

	elif [ $1 = "clean" ]; then
		cd wgshell/external

		rm -rf ./ncurses-6.2 > /dev/null 2>&1
		rm -rf ./readline-8.1 > /dev/null 2>&1 
		rm ./*.tar.gz > /dev/null 2>&1

		cd ../vtysh
		make clean
	fi

	cd $CWD
}

#
# quantum-resistant wireguard kernel module
#
build_qr_wireguard()
{
	cd $CWD
	cp wireguard/kernel/bin/wireguard_qr_ko $INST_PKG_PATH/usr/bin/qrwg/wireguard.ko
	cp wireguard/kernel/bin/wireguard_orig_ko $INST_PKG_PATH/usr/bin/qrwg/wireguard_orig.ko
	cd $CWD
}

build_wireguard_package()
{
	echo
	print_green "+-------------------------------------------------------------------+"
	print_green ">>> Building a quantum-resistant wireguard package for nanopi...     "
	print_green "+-------------------------------------------------------------------+"

	build_wireguard_ui release
	build_web_agent release
	build_vtysh release
	#build_qr_wireguard release

	print_green ">>> done."
}

clean_wireguard_package()
{
	echo
	print_green ">>> Cleaning ..."

	build_wireguard_ui clean
	build_web_agent clean
	build_vtysh clean
	#build_qr_wireguard release

	print_green ">>> done."
}

copy_base_library()
{
	cd $CWD

	aarch64-openwrt-linux-musl-strip $LIB_PATH/*
	cp -r $LIB_PATH/* $INST_PKG_PATH/usr/local/lib

	cd $CWD
}

clean_env()
{
	cd $CWD

    if [ ! -d output ]; then
		mkdir -p ./output > /dev/null 2>&1
	else
		rm -rf ./output/* > /dev/null 2>&1
	fi

	rm -rf $LIB_PATH > /dev/null 2>&1
	mkdir -p $LIB_PATH > /dev/null 2>&1

	rm -rf ./rootfs/qrwg/qrwg/assets > /dev/null 2>&1
	rm -rf ./rootfs/qrwg/usr/bin/qrwg > /dev/null 2>&1
	mkdir -p ./rootfs/qrwg/usr/bin/qrwg > /dev/null 2>&1
	rm -rf ./rootfs/qrwg/usr/local/lib > /dev/null 2>&1
	mkdir -p ./rootfs/qrwg/usr/local/lib > /dev/null 2>&1

	cp ./rootfs/scripts/start_wg.sh $INST_PKG_PATH/usr/bin/qrwg > /dev/null 2>&1
	cp ./rootfs/scripts/stop_wg.sh $INST_PKG_PATH/usr/bin/qrwg > /dev/null 2>&1
	cp ./rootfs/scripts/qrwg $INST_PKG_PATH/etc/init.d > /dev/null 2>&1
	cp ./rootfs/kernel/*.ko $INST_PKG_PATH/usr/bin/qrwg > /dev/null 2>&1

	cd $CWD
}

#
# Create a qr_wireguard_$VERSION.tar.gz file
#
create_outputs()
{
	cd $CWD

	echo
	print_green ">>> Creating a quantum-resistant wireguard package(tar.gz) ..."
	if [[ ! -f $INST_PKG_PATH/usr/bin/qrwg/web-agentd ]]; then
		print_red ">>> WARNING: making a quantum-resistant wireguard package is failed."
		exit 1
	fi

	copy_base_library

	#cd $INST_PKG_PATH
	#chmod 755 * > /dev/null 2>&1
	#cd ../..
		
	mkdir -p qr_install > /dev/null 2>&1

	cd rootfs/qrwg
	tar -cvzf qrwg_pkg.tar.gz ./*
	mv qrwg_pkg.tar.gz ../../qr_install/
	cd ../..

	cp rootfs/scripts/Install.sh qr_install/
	cp rootfs/scripts/Uninstall.sh qr_install/

	tar -cvzf qr_wireguard_$VERSION.tar.gz qr_install/
	mv qr_wireguard_$VERSION.tar.gz $OUTPUT

	rm -rf qr_install
	#rm -rf $CWD/lib

	print_green ">>> done."

	echo
	print_green ">>> OK, build result(output/ folder) is as follows."
	print_green "+---------------------------------------------------------------------+"
	ls -l $OUTPUT
	print_green "+---------------------------------------------------------------------+"
	echo

	cd $CWD
}

start_now()
{
	if [ $# -eq 0 ]; then
		echo "Usage: $0 release|clean"
		exit 0
	fi

	if [ $1 = "release" ]; then
		clean_env
		build_wireguard_package
		create_outputs
	elif [ $1 = "clean" ]; then
		clean_env
		clean_wireguard_package
	else
		echo "Usage: $0 release|clean"
	fi
}

start_now $1

#!/bin/sh

#   
# Uninstallation Shell Script for Quantum-Resistant Wireguard
#
# Copyright (c) 2024 Chunghan Yi <chunghan.yi@gmail.com>
# SPDX-License-Identifier: Apache-2.0
#

#
# You should run this script with root permission !
# ./Uninstall.sh
#

VERSION=0.09.00
BOARDTYPE="SGW"
QRWG_SERVICE_FILE=/etc/init.d/qrwg

echo "Uninstalling the quantum-resistant wireguard package for $BOARDTYPE..."

print_reboot_message()
{
	echo
	while true; do
		read -p ">>> Do you want to restart the system now?(y/n)" yn
		case $yn in
			[Yy]* ) sync; sync; reboot; break;;
			[Nn]* ) break;;
			* ) echo "Please answer yes or no.";;
		esac
	done
}

upgrade_system()
{
	echo
	#opkg update
	#opkg remove wireguard-tools
}

#
# quantum-resistant wireguard package uninstallation
#
remove_pkg()
{
	if [ -r $QRWG_SERVICE_FILE ]; then
		service qrwg stop > /dev/null 2>&1
		/etc/init.d/qrwg disable > /dev/null 2>&1

		# Restore original firewall settings
		if [ -r /usr/bin/qrwg/.firewall.bak ]; then
			cp /usr/bin/qrwg/.firewall.bak /etc/config/firewall
		fi

		cp /usr/bin/qrwg/wireguard_orig.ko /lib/modules/6.1.63/wireguard.ko > /dev/null 2>&1

		rm -rf /qrwg > /dev/null 2>&1
		rm /etc/init.d/qrwg > /dev/null 2>&1
		rm /root/.profile > /dev/null 2>&1
		rm -rf /usr/bin/qrwg > /dev/null 2>&1
		rm -rf /usr/local/lib > /dev/null 2>&1
		rm -rf /db > /dev/null 2>&1
		rm -rf /root/db > /dev/null 2>&1

		uci commit firewall > /dev/null 2>&1

		upgrade_system

		sync; sync
		echo "OK, done."
		print_reboot_message
	else
		echo "Warning: quantum-resistant wireguard package is not installed."
	fi
}

remove_pkg

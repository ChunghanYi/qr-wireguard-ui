#!/bin/sh

#
# Installation Shell Script for Quantum-Resistant Wireguard
#
# Copyright (c) 2024 Chunghan Yi <chunghan.yi@gmail.com>
# SPDX-License-Identifier: Apache-2.0
#

#
# You should run this script with root permission !
# ./Install.sh
#

VERSION=0.09.00
BOARDTYPE="SGW"
QRWG_SERVICE_FILE=/etc/init.d/qrwg

print_greetings()
{
	echo
}

upgrade_system()
{
	echo
	opkg update
	opkg install wireguard-tools
}

update_system_config()
{
	# Backup original firewall settings
	if [ ! -r /usr/bin/qrwg/.firewall.bak ]; then
		cp /etc/config/firewall /usr/bin/qrwg/.firewall.bak
	fi

	# Change hostname
	uci set system.@system[0].hostname='sgw' > /dev/null 2>&1
	uci set system.@system[0].zonename='Asia/Seoul' > /dev/null 2>&1
	uci set system.@system[0].timezone='KST-9' > /dev/null 2>&1
	uci commit system > /dev/null 2>&1

	# Add network interface 
	uci set network.wg0=interface > /dev/null 2>&1
	uci set network.wg0.ifname='wg0' > /dev/null 2>&1
	uci set network.wg0.device='wg0' > /dev/null 2>&1
	uci set network.wg0.proto='none' > /dev/null 2>&1
	uci commit network > /dev/null 2>&1

	#for Kernel WireGuard ----------------------------------------
	# Add the firewall zone
	uci add firewall zone > /dev/null 2>&1
	uci set firewall.@zone[-1].name='wg' > /dev/null 2>&1
	uci set firewall.@zone[-1].input='ACCEPT' > /dev/null 2>&1
	uci set firewall.@zone[-1].forward='ACCEPT' > /dev/null 2>&1
	uci set firewall.@zone[-1].output='ACCEPT' > /dev/null 2>&1
	uci set firewall.@zone[-1].masq='1' > /dev/null 2>&1
	uci set firewall.@zone[-1].mtu_fix='1' > /dev/null 2>&1

	# Add the wireguard interface to it
	uci set firewall.@zone[-1].network='wg0' > /dev/null 2>&1

	# Forward WAN and LAN traffic to/from it
	uci add firewall forwarding > /dev/null 2>&1
	uci set firewall.@forwarding[-1].src='wg' > /dev/null 2>&1
	uci set firewall.@forwarding[-1].dest='wan' > /dev/null 2>&1
	uci add firewall forwarding > /dev/null 2>&1
	uci set firewall.@forwarding[-1].src='wg' > /dev/null 2>&1
	uci set firewall.@forwarding[-1].dest='lan' > /dev/null 2>&1
	uci add firewall forwarding > /dev/null 2>&1
	uci set firewall.@forwarding[-1].src='lan' > /dev/null 2>&1
	uci set firewall.@forwarding[-1].dest='wg' > /dev/null 2>&1
	uci add firewall forwarding > /dev/null 2>&1
	uci set firewall.@forwarding[-1].src='wan' > /dev/null 2>&1
	uci set firewall.@forwarding[-1].dest='wg' > /dev/null 2>&1
	#------------------------------------------------------------
	uci commit firewall > /dev/null 2>&1

	/etc/init.d/firewall restart > /dev/null 2>&1
}

install_pkg()
{
	echo "Installing the quantum-resistant wireguard package for $BOARDTYPE ..."

	if [ -r $QRWG_SERVICE_FILE ]; then
		echo ""
		echo "Warning: qr-wireguard package has already been installed."
		echo "Please uninstall it first if you want to reinstall it !"
		exit 1
	else
		/etc/init.d/qrwg disable > /dev/null 2>&1

		tar xvzf ./qrwg_pkg.tar.gz -C / > /dev/null 2>&1

		cp /usr/bin/qrwg/wireguard_qr.ko /lib/modules/6.1.63/wireguard.ko > /dev/null 2>&1
		mv /root/profile /root/.profile > /dev/null 2>&1

		/etc/init.d/qrwg enable > /dev/null 2>&1

		update_system_config

		sync; sync
		echo "OK, done."
	fi
}

print_ok_message()
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

print_greetings
upgrade_system
install_pkg
print_ok_message

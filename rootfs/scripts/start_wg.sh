#!/bin/sh
#
# Script for starting wireguard
#

export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
export PATH=/usr/bin/qrwg:$PATH

ip link add dev wg0 type wireguard

#wg key generation w/ wg tool
if [ ! -r /config/publickey ]; then
	/usr/bin/qrwg/vtysh -e "wg regenerate-key"
fi

/usr/bin/qrwg/vtysh -b > /dev/null 2>&1

#Run the wireguard web agent
/usr/bin/qrwg/web-agentd -d &
sleep 1

#Run the wireguard web ui
if [ ! -r /etc/wireguard/wg0.conf ]; then
	mkdir -p /etc/wireguard > /dev/null 2>&1
else
	echo > /etc/wireguard/wg0.conf
fi
/usr/bin/qrwg/wireguard-ui &
sleep 2

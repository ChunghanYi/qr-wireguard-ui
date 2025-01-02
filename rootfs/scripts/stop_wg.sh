#!/bin/sh
#
# Script for stopping wireguard
#

ip link delete dev wg0 > /dev/null 2>&1

killall web-agentd > /dev/null 2>&1
sleep 1
killall wireguard-ui > /dev/null 2>&1
sleep 2
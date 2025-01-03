/*
 * IP Control
 * Copyright (c) 2024 Chunghan Yi <chunghan.yi@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "command.h"
#include "vtysh_config.h"
#include <unistd.h>

DEFUN (show_ip_address,
       show_ip_address_cmd,
       "show ip address",
       SHOW_STR
       IP_STR
       "IP address list\n")
{
	char *myargv[2];

	myargv[0] = "addr";
	return cmd_execute_system_command("ip", 1, myargv);
}

DEFUN (show_ip_config,
       show_ip_config_cmd,
       "show ip config",
       SHOW_STR
       IP_STR
       "IP config\n")
{
	char *myargv[2];

	myargv[0] = "-a";
	return cmd_execute_system_command("ifconfig", 1, myargv);
}

DEFUN (show_ip_config_name,
       show_ip_config_name_cmd,
       "show ip config WORD",
       SHOW_STR
       IP_STR
       "IP config name\n"
	   "the interface name\n")
{
	return cmd_execute_system_command("ifconfig", 1, argv);
}

DEFUN(show_ip_route, 
	  show_ip_route_cmd,
      "show ip route", 
      SHOW_STR
      IP_STR
      "IP routing table\n")
{
	char *myargv[2];

	myargv[0] = "-n";
	return cmd_execute_system_command("route", 1, myargv);
}

DEFUN (ip_address, ip_address_cmd, 
       "ip address ETHNAME A.B.C.D A.B.C.D", 
       IP_STR
       "config the ip address\n"
       "interface name(lan|wan|wg0|wg1)\n"
       "ip address e.g. x.x.x.x\n"
       "ip netmask  e.g. 255.255.0.0\n")
{
	char *myargv[10];
	char line[1024];

	if (strcmp(argv[0], "lan") &&    /* LAN: br-lan(eth1) */
			strcmp(argv[0], "wan") &&    /* WAN: eth0 */
			strncmp(argv[0], "wg0", 3) &&
			strncmp(argv[0], "wg1", 3)) {
		vty_out(vty, "%% Not supported interface(%s).\n", argv[0]);
		return CMD_WARNING;
	}

	sprintf(line, "ip address %s", argv[0]);
	config_del_line_byleft(config_top, line);
	sprintf(line, "ip address %s %s %s", argv[0], argv[1], argv[2]);
	config_add_line(config_top, line);

	ENSURE_CONFIG(vty);

	if (!strcmp(argv[0], "lan")) {
		/* Change the LAN configurations */
		sprintf(line, "uci set network.lan.proto='static' > /dev/null 2>&1");  /* set: set or add */
		system(line);

		sprintf(line, "uci set network.lan.ipaddr='%s' > /dev/null 2>&1", argv[1]);
		system(line);

		sprintf(line, "uci set network.lan.netmask='%s' > /dev/null 2>&1", argv[2]);
		system(line);

		sprintf(line, "uci commit network > /dev/null 2>&1");
		system(line);

	} else if (!strcmp(argv[0], "wan")) {
		/* Change the LAN configurations */
		sprintf(line, "uci set network.wan.proto='static' > /dev/null 2>&1");
		system(line);

		sprintf(line, "uci set network.wan.ipaddr='%s' > /dev/null 2>&1", argv[1]);
		system(line);

		sprintf(line, "uci set network.wan.netmask='%s' > /dev/null 2>&1", argv[2]);
		system(line);

		sprintf(line, "uci commit network > /dev/null 2>&1");
		system(line);

		sprintf(line, "/etc/init.d/network restart > /dev/null 2>&1");
		system(line);

	} else if (!strncmp(argv[0], "wg0", 3) || !strncmp(argv[0], "wg1", 3)) {

	} else {
		vty_out(vty, "%% Not supported interface(%s).\n", argv[0]);
		return CMD_WARNING;
	}

	myargv[0] = argv[0];
	myargv[1] = argv[1];
	myargv[2] = "netmask";
	myargv[3] = argv[2];
	cmd_execute_system_command("ifconfig", 4, myargv);

	if (!strcmp(argv[0], "lan")) {
		vty_out(vty, "You should reboot the system after 'write' to apply your changes.\n");
	}

	return CMD_SUCCESS;
}

DEFUN (no_ip_address, no_ip_address_cmd, 
       "no ip address ETHNAME", 
       NO_STR
       IP_STR
       "Disable the ip address\n"
       "config the ip address\n"
       "interface name(lan|wan|wg0|wg1)\n")
{
	char *myargv[10];
	char line[1024];

	if (strcmp(argv[0], "lan") &&  /* LAN: br-lan(eth1) */
		strcmp(argv[0], "wan") &&  /* WAN: eth0 */
		strncmp(argv[0], "wg0", 3) &&
		strncmp(argv[0], "wg1", 3)) {
		vty_out(vty, "%% Not supported interface(%s).\n", argv[0]);
		return CMD_WARNING;
	}

	vty_out(vty, "Do you really want to down the ip address ? (y/n):\n");
	if (fgets(line, sizeof(line), stdin)) {
		if (line[0] != 'y') {
			vty_out(vty, "no ip address %s cancelled.\n", argv[0]);
			return CMD_SUCCESS;
		}
	}

	sprintf(line, "ip address %s", argv[0]);
	config_del_line_byleft(config_top, line);

	if (!strcmp(argv[0], "lan")) {
		sprintf(line, "uci delete network.lan.proto > /dev/null 2>&1");
		system(line);

		sprintf(line, "uci delete network.lan.ipaddr > /dev/null 2>&1");
		system(line);

		sprintf(line, "uci delete network.lan.netmask > /dev/null 2>&1");
		system(line);

		sprintf(line, "uci commit network > /dev/null 2>&1");
		system(line);

	} else if (!strcmp(argv[0], "wan")) {
		sprintf(line, "uci delete network.wan.proto > /dev/null 2>&1");
		system(line);

		sprintf(line, "uci delete network.wan.ipaddr > /dev/null 2>&1");
		system(line);

		sprintf(line, "uci delete network.wan.netmask > /dev/null 2>&1");
		system(line);

		sprintf(line, "uci commit network > /dev/null 2>&1");
		system(line);

		sprintf(line, "/etc/init.d/network restart > /dev/null 2>&1");
		system(line);

	} else if (!strncmp(argv[0], "wg0", 3) || !strncmp(argv[0], "wg1", 3)) {

	} else {
		vty_out(vty, "%% Not supported interface(%s).\n", argv[0]);
		return CMD_WARNING;
	}

	myargv[0] = argv[0];
	myargv[1] = "down";
	cmd_execute_system_command("ifconfig", 2, myargv);

	if (!strcmp(argv[0], "lan")) {
		vty_out(vty, "You should reboot the system after 'write' to apply your changes.\n");
	}
	return CMD_SUCCESS;
}

DEFUN (ip_address_dhcp, ip_address_dhcp_cmd, 
       "ip address ETHNAME dhcp", 
       IP_STR
       "config the ip address\n"
       "interface name(wan)\n"
       "DHCP mode\n")
{
	char line[1024];

	if (strcmp(argv[0], "wan")) {
		vty_out(vty, "%% Not supported interface(%s).\n", argv[0]);
		return CMD_WARNING;
	}

	sprintf(line, "ip address %s", argv[0]);
	config_del_line_byleft(config_top, line);
	sprintf(line, "ip address %s dhcp", argv[0]);
	config_add_line(config_top, line);

	ENSURE_CONFIG(vty);

	if (!strcmp(argv[0], "wan")) {
		sprintf(line, "uci delete network.wan.ipaddr > /dev/null 2>&1");
		system(line);

		sprintf(line, "uci delete network.wan.netmask > /dev/null 2>&1");
		system(line);

		sprintf(line, "uci set network.wan.proto=dhcp > /dev/null 2>&1");  /* set: set or add */
		system(line);

		sprintf(line, "uci commit network > /dev/null 2>&1");
		system(line);

		sprintf(line, "/etc/init.d/network restart > /dev/null 2>&1");
		system(line);
	}

	return CMD_SUCCESS;
}

//route add -net 192.168.56.0 netmask 255.255.255.0 gw 192.168.54.1
DEFUN (ip_route, ip_route_cmd, 
       "ip route A.B.C.D A.B.C.D A.B.C.D ETHNAME", 
       IP_STR
       "config the ip route(temporary)\n"
       "Destination address\n"
       "Destination adress netmask e.g. 255.255.255.0\n"
       "Nexthot address\n"
       "interface name(lan or wan)\n")
{
	char line[1024];

	if (strcmp(argv[3], "lan") &&    /* LAN: br-lan(eth1) */
		strcmp(argv[3], "wan") &&    /* WAN: eth0 */
		strncmp(argv[3], "wg0", 3) &&
		strncmp(argv[3], "wg1", 3)) {
		vty_out(vty, "%% Not supported interface(%s).\n", argv[3]);
		return CMD_WARNING;
	}

	sprintf(line, "ip route %s %s %s %s", argv[0], argv[1], argv[2], argv[3]);
	config_del_line(config_top, line);
	config_add_line(config_top, line);

	ENSURE_CONFIG(vty);

	sprintf(line, "uci add network route > /dev/null 2>&1");
	system(line);

	sprintf(line, "uci set network.@route[-1].interface='%s' > /dev/null 2>&1", argv[3]);
	system(line);

	sprintf(line, "uci set network.@route[-1].target='%s' > /dev/null 2>&1", argv[0]);
	system(line);

	sprintf(line, "uci set network.@route[-1].netmask='%s' > /dev/null 2>&1", argv[1]);
	system(line);

	sprintf(line, "uci set network.@route[-1].gateway='%s' > /dev/null 2>&1", argv[2]);
	system(line);

	sprintf(line, "uci commit network > /dev/null 2>&1");
	system(line);

	sprintf(line, "/etc/init.d/network restart > /dev/null 2>&1");
	system(line);

	return CMD_SUCCESS;
}

DEFUN (no_ip_route, no_ip_route_cmd, 
       "no ip route A.B.C.D A.B.C.D", 
       NO_STR
       IP_STR
       "Disable the ip route\n"
       "Destination address\n"
       "Destination adress netmask e.g. 255.255.255.0\n")
{
	char line[1024];

	sprintf(line, "ip route %s %s", argv[0], argv[1]);
	config_del_line_byleft(config_top, line);

	sprintf(line, "uci delete network.@route[-1] > /dev/null 2>&1");
	system(line);

	sprintf(line, "uci commit network > /dev/null 2>&1");
	system(line);

	sprintf(line, "/etc/init.d/network restart > /dev/null 2>&1");
	system(line);

	return CMD_SUCCESS;
}

/*
 * search lan
 * nameserver 127.0.0.1
 * nameserver ::1
 */
DEFUN (config_dns, config_dns_cmd, 
       "nameserver A.B.C.D A.B.C.D", 
       "Config the dns server\n"
       "the first dns server ip\n"
       "the second dns server ip\n")
{
	FILE *fp = NULL;
	config_del_line_byleft(config_top, "nameserver");
	config_add_line(config_top, "nameserver %s %s", argv[0], argv[1]);

	ENSURE_CONFIG(vty);

	fp = fopen("/etc/resolv.conf", "w");
	if (fp == NULL) {
		vty_out(vty, "open the resolv.conf error\n");
		return CMD_ERR_NOTHING_TODO;
	}
	fprintf(fp, "nameserver %s\n", argv[0]);
	fprintf(fp, "nameserver %s\n", argv[1]);
	fclose(fp);

	return CMD_SUCCESS;
}

DEFUN (config_no_dns, config_no_dns_cmd, 
       "no nameserver",
       NO_STR
       "Remove the dns server\n")
{
	FILE *fp = NULL;
	config_del_line_byleft(config_top, "nameserver");

	ENSURE_CONFIG(vty);

	fp = fopen("/etc/resolv.conf", "w");
	if (fp == NULL) {
		vty_out(vty, "open the resolv.conf error\n");
		return CMD_ERR_NOTHING_TODO;
	}
	fprintf(fp, "nameserver 127.0.0.1\n");
	fclose(fp);

	return CMD_SUCCESS;
}

int cmd_ip_init()
{
	cmd_install_element (ENABLE_NODE, &show_ip_address_cmd);
	cmd_install_element (ENABLE_NODE, &show_ip_config_cmd);
	cmd_install_element (ENABLE_NODE, &show_ip_config_name_cmd);
	cmd_install_element (ENABLE_NODE, &show_ip_route_cmd);

	cmd_install_element (CONFIG_NODE, &show_ip_address_cmd);
	cmd_install_element (CONFIG_NODE, &show_ip_config_cmd);
	cmd_install_element (CONFIG_NODE, &show_ip_config_name_cmd);
	cmd_install_element (CONFIG_NODE, &show_ip_route_cmd);

	cmd_install_element (CONFIG_NODE, &ip_address_cmd);
	cmd_install_element (CONFIG_NODE, &no_ip_address_cmd);
	cmd_install_element (CONFIG_NODE, &ip_address_dhcp_cmd);
	cmd_install_element (CONFIG_NODE, &ip_route_cmd);
	cmd_install_element (CONFIG_NODE, &no_ip_route_cmd);
	cmd_install_element (CONFIG_NODE, &config_dns_cmd);
	cmd_install_element (CONFIG_NODE, &config_no_dns_cmd);

	return 0;
}

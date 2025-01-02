/*
 * sfirewall(stateful firewall) - packet filter/nat
 * Copyright (c) 2024 Chunghan Yi <chunghan.yi@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "command.h"
#include "vtysh_config.h"
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>

/*
 * sfirewall nat portmap NUM (vpn|wan) internal-ip proto port1 port2
 * sfirewall nat portmap apply
 *
 * sfirewall filter NUM lan2wan|wan2lan in|out|fw insert|append permit|deny|reject tcp|udp|icmp
 *                  sip/smask sport dip/dmask dport mon|tue|wed|thu|fri|sat|sun|any 21:00 09:00
 * sfirewall filter apply
 *
 * show sfirewall (all|nat|filter)
 */

void apply_firewall_rules(void)
{
	char *orig_firewall_rule_file = "/etc/config/firewall.ORG";
	char *sfirewall_filter_rule_file = "/etc/config/sfirewall_filter";
	char *sfirewall_macfilter_rule_file = "/etc/config/sfirewall_mac";
	char *sfirewall_nat_rule_file = "/etc/config/sfirewall_nat";
	struct stat sb;

	// New firewall = firewall.ORG + (sfirewall_filter + sfirewall_nat + sfirewall_mac)
	if (stat(orig_firewall_rule_file, &sb) != 0) {
		system("cp /etc/config/firewall /etc/config/firewall.ORG > /dev/null 2>&1");
	}

	// if not exist, let's create an empty file.
	{
		if (stat(sfirewall_filter_rule_file, &sb) != 0)
			system("touch /etc/config/sfirewall_filter > /dev/null 2>&1");
		if (stat(sfirewall_macfilter_rule_file, &sb) != 0)
			system("touch /etc/config/sfirewall_mac > /dev/null 2>&1");
		if (stat(sfirewall_nat_rule_file, &sb) != 0)
			system("touch /etc/config/sfirewall_nat > /dev/null 2>&1");
	}

	// Regenerate the /etc/config/sfirewall file with sfirewall_filter/_mac/_nat files.
	system("cat /etc/config/sfirewall_filter /etc/config/sfirewall_mac /etc/config/sfirewall_nat > /tmp/sfirewall");
	system("mv /tmp/sfirewall /etc/config > /dev/null 2>&1");

	// Regenerate the /etc/config/firewall file.
	system("cat /etc/config/firewall.ORG /etc/config/sfirewall > /tmp/firewall");
	
	system("mv /tmp/firewall /etc/config > /dev/null 2>&1");

	// Apply a new file rules.
	system("uci commit firewall > /dev/null 2>&1");
	system("/etc/init.d/firewall restart > /dev/null 2>&1");
}

#if 0
DEFUN (sfirewall_enable,
		sfirewall_enable_cmd,
		"sfirewall enable",
		"Configure smart firewall rules\n"
		"Enable default rules\n")
{
	char szInfo[1024];

	sprintf(szInfo, "sfirewall enable");
	config_del_line(config_top, szInfo);
	config_add_line(config_top, szInfo);

	ENSURE_CONFIG(vty);

	sprintf(szInfo, "/qrwg/config/fw.sh > /dev/null 2>&1");
	system(szInfo);

	return CMD_SUCCESS;
}

DEFUN (no_sfirewall_enable,
		no_sfirewall_enable_cmd,
		"no sfirewall enable",
		NO_STR
		"Configure smart firewall rules\n"
		"Enable default rules\n")
{
	char szInfo[1024];

	sprintf(szInfo, "sfirewall enable");
	config_del_line(config_top, szInfo);

	ENSURE_CONFIG(vty);

	vty_out(vty, "You should reboot the system after 'write' to apply your changes.\n");

	return CMD_SUCCESS;
}
#endif

DEFUN (sfirewall_nat_portmap, sfirewall_nat_portmap_cmd,
		"sfirewall nat portmap NUM (vpn|wan) A.B.C.D (tcp|udp|all) PORT PORT",
		"Configure smart firewall rules\n"
		"Add NAT rules\n"
		"Port forwarding\n"
		"Index number(Rule order) e.g. 10\n"
		"VPN interface(e.g. wg0)\n"
		"WAN interface\n"
		"Internal ip address e.g. 192.168.5.200\n"
		"TCP Protocol\n"
		"UDP Protocol\n"
		"All(tcp/udp/icmp) Protocols\n"
		"port1 e.g. 8080 or 0(= any port)\n"
		"port2 e.g. 80 or 0(= any port)\n")
{
	char line[1024];

	if (!isdigit(argv[0][0])) {
		vty_out(vty, "%% Invalid format '%s'\n", argv[0]);
		return CMD_ERR_NOTHING_TODO;
	}

	if (!strcmp(argv[3], "all") && strcmp(argv[4], "0")) {
		vty_out(vty, "%% Invalid port number '%s'\n", argv[4]);
		return CMD_ERR_NOTHING_TODO;
	}

	if (!strcmp(argv[3], "all") && strcmp(argv[5], "0")) {
		vty_out(vty, "%% Invalid port number '%s'\n", argv[5]);
		return CMD_ERR_NOTHING_TODO;
	}

	sprintf(line, "sfirewall nat portmap %s %s %s %s %s %s",
			argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
	config_del_line(config_top, line);
	config_add_line(config_top, line);

	ENSURE_CONFIG(vty);

	return 0;
}

DEFUN (no_sfirewall_nat_portmap, no_sfirewall_nat_portmap_cmd,
		"no sfirewall nat portmap NUM (vpn|wan) A.B.C.D (tcp|udp|all) PORT PORT",
		NO_STR
		"Configure smart firewall rules\n"
		"Add NAT rules\n"
		"Port forwarding\n"
		"Index number(Rule order) e.g. 10\n"
		"VPN interface(e.g. wg0)\n"
		"WAN interface\n"
		"Internal ip address e.g. 192.168.5.200\n"
		"TCP Protocol\n"
		"UDP Protocol\n"
		"All(tcp/udp/icmp) Protocols\n"
		"port e.g. 8080 or 0(= any port)\n"
		"port e.g. 80 or 0(= any port)\n")
{
	char line[1024];

	if (!isdigit(argv[0][0])) {
		vty_out(vty, "%% Invalid format '%s'\n", argv[0]);
		return CMD_ERR_NOTHING_TODO;
	}

	if (!strcmp(argv[3], "all") && strcmp(argv[4], "0")) {
		vty_out(vty, "%% Invalid port number '%s'\n", argv[4]);
		return CMD_ERR_NOTHING_TODO;
	}

	if (!strcmp(argv[3], "all") && strcmp(argv[5], "0")) {
		vty_out(vty, "%% Invalid port number '%s'\n", argv[5]);
		return CMD_ERR_NOTHING_TODO;
	}

	sprintf(line, "sfirewall nat portmap %s %s %s %s %s %s",
			argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
	config_del_line(config_top, line);

	ENSURE_CONFIG(vty);

	return 0;
}

DEFUN (sfirewall_nat_portmap_apply,
        sfirewall_nat_portmap_apply_cmd,
        "sfirewall nat portmap apply",
        "Configure smart firewall rules\n"
		"Add NAT rules\n"
		"Port forwarding\n"
        "Apply\n")
{
	char *portmap_config_file = CONFIG_DIR "/" "portmap.txt";
	char *sfirewall_nat_rule_file = "/etc/config/sfirewall_nat";
	FILE *rfp, *wfp;
	char xbuf[1024], *s;
	char param[6][32]; 
	int portmap_len = strlen("sfirewall nat portmap");

	wfp = fopen(sfirewall_nat_rule_file, "w");
	if (!wfp)
		return CMD_WARNING;
	fprintf(wfp, "\n");

	rfp = fopen(portmap_config_file, "r");
	if (!rfp)
		return CMD_WARNING;

	memset(xbuf, 0, sizeof(xbuf));
	while (fgets(xbuf, sizeof(xbuf), rfp)) {
		xbuf[strlen(xbuf)-1] = '\0';

		// ex: sfirewall nat portmap 10 wan 192.168.8.100 tcp 8080 80
		if (!strncmp(xbuf, "sfirewall nat portmap", portmap_len)) {
			s = strtok(&xbuf[portmap_len+1], " "); // 10
			if (s == NULL) continue;
			sprintf(param[0], "%s", s);

			s = strtok(NULL, " ");      // wan
			if (s == NULL) continue;
			sprintf(param[1], "%s", s);

			s = strtok(NULL, " ");      // 192.168.8.100
			if (s == NULL) continue;
			sprintf(param[2], "%s", s);

			s = strtok(NULL, " ");      // tcp
			if (s == NULL) continue;
			sprintf(param[3], "%s", s);

			s = strtok(NULL, " ");      // 8080
			if (s == NULL) continue;
			sprintf(param[4], "%s", s);

			s = strtok(NULL, " ");      // 80
			if (s == NULL) continue;
			sprintf(param[5], "%s", s);

			// ---------------------------------------------
			
			fprintf(wfp, "config redirect\n");
			fprintf(wfp, "       option target          'DNAT'\n");
			fprintf(wfp, "       option src             '%s'\n", param[1]);
			fprintf(wfp, "       option dest            'lan'\n");
			fprintf(wfp, "       option proto           '%s'\n", param[3]);
			if (strcmp(param[4], "0"))  // any port
				fprintf(wfp, "       option src_dport       '%s'\n", param[4]);
			fprintf(wfp, "       option dest_ip         '%s'\n", param[2]);
			if (strcmp(param[5], "0"))  // any port
				fprintf(wfp, "       option dest_port       '%s'\n", param[5]);
			fprintf(wfp, "       option enabled         '1'\n");
			fprintf(wfp, "\n");
		}
		memset(xbuf, 0, sizeof(xbuf));
	}
	fclose(wfp);
	fclose(rfp);

	apply_firewall_rules();

    return CMD_SUCCESS;
}

DEFUN (sfirewall_filter_rules, sfirewall_filter_rules_cmd,
		"sfirewall filter NUM (lan2wan|wan2lan) (in|out|fw) (insert|append) (permit|deny|reject) (tcp|udp|icmp) A.B.C.D/E PORT A.B.C.D/E PORT DAY FROMTIME TOTIME",
		"Configure smart firewall rules\n"
		"Add filter rules\n"
		"Index number(Rule order) e.g. 10\n"
		"from lan to wan zone\n"
		"from wan to lan zone\n"
		"Input direction\n"
		"Output direction\n"
		"Forward direction\n"
		"Insert at the front\n"
		"Append to the back\n"
		"Permit\n"
		"Deny\n"
		"Reject\n"
		"TCP Protocol\n"
		"UDP Protocol\n"
		"ICMP Protocol\n"
		"Source ip address/netmask(CIDR) e.g. 192.168.5.200/32\n"
		"Source port e.g. 8080 or any\n"
		"Destination ip address/netmask(CIDR) e.g. 0.0.0.0/0\n"
		"Destination port e.g. 80\n"
		"Limit day e.g. mon|tue|wed|thu|fri|sat|sun or any\n"
		"Start(from) time(00:00 ~ 23:59) e.g. 09:00 or any\n"
		"Stop(to) time(00:00 ~ 23:59) e.g. 21:00 or any\n")
{
	char line[1024];
	char sip[256], dip[256];
	int sport = 0, dport = 0;
	char day[128];
	int day_count = 0;
	char time_duration[128];
	char *s;

	if (!isdigit(argv[0][0])) {
		vty_out(vty, "%% Invalid format '%s'\n", argv[0]);
		return CMD_ERR_NOTHING_TODO;
	}

	if (strcmp(argv[1], "lan2wan") && strcmp(argv[1], "wan2lan")) {
		vty_out(vty, "%% Not supported option(%s).\n", argv[1]);
		return CMD_WARNING;
	}

	if (strcmp(argv[2], "in") && strcmp(argv[2], "out") && strcmp(argv[2], "fw")) {
		vty_out(vty, "%% Not supported option(%s).\n", argv[2]);
		return CMD_WARNING;
	}

	if (strcmp(argv[3], "insert") && strcmp(argv[3], "append")) {
		vty_out(vty, "%% Not supported option(%s).\n", argv[3]);
		return CMD_WARNING;
	}

	if (strcmp(argv[4], "permit") && strcmp(argv[4], "deny") && strcmp(argv[4], "reject")) {
		vty_out(vty, "%% Not supported option(%s).\n", argv[4]);
		return CMD_WARNING;
	}

	if (!strcmp(argv[7], "any")) {
		sport = 0;
	} else {
		if (!isdigit(argv[7][0])) {
			vty_out(vty, "%% Invalid port number '%s'\n", argv[7]);
			return CMD_ERR_NOTHING_TODO;
		}
		sport = atoi(argv[7]);
		if (sport <= 0 || sport > 65535) {
			vty_out(vty, "%% Invalid port number '%s'\n", argv[7]);
			return CMD_ERR_NOTHING_TODO;
		}
	}

	if (!strcmp(argv[9], "any")) {
		dport = 0;
	} else {
		if (!isdigit(argv[9][0])) {
			vty_out(vty, "%% Invalid port number '%s'\n", argv[9]);
			return CMD_ERR_NOTHING_TODO;
		}
		dport = atoi(argv[9]);
		if (dport <= 0 || dport > 65535) {
			vty_out(vty, "%% Invalid port number '%s'\n", argv[9]);
			return CMD_ERR_NOTHING_TODO;
		}
	}

	/* ip address/netmask sanity check */
	if (!isdigit(argv[6][0])) {
		vty_out(vty, "%% Invalid ip address '%s'\n", argv[6]);
		return CMD_ERR_NOTHING_TODO;
	}
	sprintf(sip, "%s", argv[6]);
	s = strstr(sip, "/");
	if (s) {
		if (strlen(s) > 3) {
			vty_out(vty, "%% Invalid netmask '%s'\n", &s[1]);
			return CMD_ERR_NOTHING_TODO;
		}
	} else {
		vty_out(vty, "%% Invalid ip address/netmask '%s'\n", argv[6]);
		return CMD_ERR_NOTHING_TODO;
	}

	if (!isdigit(argv[8][0])) {
		vty_out(vty, "%% Invalid ip address '%s'\n", argv[8]);
		return CMD_ERR_NOTHING_TODO;
	}
	sprintf(dip, "%s", argv[8]);
	s = strstr(dip, "/");
	if (s) {
		if (strlen(s) > 3) {
			vty_out(vty, "%% Invalid netmask '%s'\n", &s[1]);
			return CMD_ERR_NOTHING_TODO;
		}
	} else {
		vty_out(vty, "%% Invalid ip address/netmask '%s'\n", argv[8]);
		return CMD_ERR_NOTHING_TODO;
	}

	/* Sanity check for day string(mon|tue|wed|thu|fri|sat|sun) */
	if (strcmp(argv[10], "any")) {
		sprintf(day, "%s", argv[10]);
		s = strtok(day, "|");
		while (s) {
			if (strlen(s) > 3 || strlen(s) < 3) {
				vty_out(vty, "%% Invalid day value '%s'\n", argv[10]);
				return CMD_ERR_NOTHING_TODO;
			}
			day_count++;
			s = strtok(NULL, "|");
		}
		if (day_count > 7) {
			vty_out(vty, "%% Invalid day value '%s'\n", argv[10]);
			return CMD_ERR_NOTHING_TODO;
		}
	}

	/* Sanity check for from-time string(08:00) */
	if (strcmp(argv[11], "any")) {
		sprintf(time_duration, "%s", argv[11]);
		s = strtok(time_duration, ":");
		if (s == NULL) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[11]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (strlen(s) > 2 || strlen(s) < 2) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[11]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (atoi(s) > 23 || atoi(s) < 0) {
			vty_out(vty, "%% Invalid time(hour) value '%s'\n", argv[11]);
			return CMD_ERR_NOTHING_TODO;
		}
		s = strtok(NULL, ":");
		if (s == NULL) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[11]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (strlen(s) > 2 || strlen(s) < 2) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[11]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (atoi(s) > 59 || atoi(s) < 0) {
			vty_out(vty, "%% Invalid time(minute) value '%s'\n", argv[11]);
			return CMD_ERR_NOTHING_TODO;
		}
	}

	/* Sanity check for to-time string(21:00) */
	if (strcmp(argv[12], "any")) {
		sprintf(time_duration, "%s", argv[12]);
		s = strtok(time_duration, ":");
		if (s == NULL) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[12]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (strlen(s) > 2 || strlen(s) < 2) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[12]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (atoi(s) > 23 || atoi(s) < 0) {
			vty_out(vty, "%% Invalid time(hour) value '%s'\n", argv[12]);
			return CMD_ERR_NOTHING_TODO;
		}
		s = strtok(NULL, ":");
		if (s == NULL) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[12]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (strlen(s) > 2 || strlen(s) < 2) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[12]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (atoi(s) > 59 || atoi(s) < 0) {
			vty_out(vty, "%% Invalid time(minute) value '%s'\n", argv[12]);
			return CMD_ERR_NOTHING_TODO;
		}
	}

	/* to prevent duplicate rules for argv[3]: insert|append */
	if (!strcmp(argv[3], "append")) {
		sprintf(line, "sfirewall filter %s %s %s %s %s %s %s %s %s %s %s %s %s",
				argv[0], argv[1], argv[2], "insert", argv[4], argv[5], argv[6],
				argv[7], argv[8], argv[9], argv[10], argv[11], argv[12]);
		config_del_line(config_top, line);
	} else if (!strcmp(argv[3], "insert")) {
		sprintf(line, "sfirewall filter %s %s %s %s %s %s %s %s %s %s %s %s %s",
				argv[0], argv[1], argv[2], "append", argv[4], argv[5], argv[6],
				argv[7], argv[8], argv[9], argv[10], argv[11], argv[12]);
		config_del_line(config_top, line);
	}

	sprintf(line, "sfirewall filter %s %s %s %s %s %s %s %s %s %s %s %s %s",
		argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8],
		argv[9], argv[10], argv[11], argv[12]);
	config_del_line(config_top, line);
	config_add_line(config_top, line);

	ENSURE_CONFIG(vty);

	return 0;
}

DEFUN (no_sfirewall_filter_rules, no_sfirewall_filter_rules_cmd,
		"no sfirewall filter NUM (lan2wan|wan2lan) (in|out|fw) (insert|append) (permit|deny|reject) (tcp|udp|icmp) A.B.C.D/E PORT A.B.C.D/E PORT DAY FROMTIME TOTIME",
		NO_STR
		"Configure smart firewall rules\n"
		"Add filter rules\n"
		"Index number(Rule order) e.g. 10\n"
		"from lan to wan zone\n"
		"from wan to lan zone\n"
		"Input direction\n"
		"Output direction\n"
		"Forward direction\n"
		"Insert at the front\n"
		"Append to the back\n"
		"Permit\n"
		"Deny\n"
		"Reject\n"
		"TCP Protocol\n"
		"UDP Protocol\n"
		"ICMP Protocol\n"
		"Source ip address/netmask(CIDR) e.g. 192.168.5.200/32\n"
		"Source port e.g. 8080 or any\n"
		"Destination ip address/netmask(CIDR) e.g. 0.0.0.0/0\n"
		"Destination port e.g. 80\n"
		"Limit day e.g. mon|tue|wed|thu|fri|sat|sun or any\n"
		"Start(from) time(00:00 ~ 23:59) e.g. 09:00 or any\n"
		"Stop(to) time(00:00 ~ 23:59) e.g. 21:00 or any\n")
{
	char line[1024];
	char sip[256], dip[256];
	int sport = 0, dport = 0;
	char day[128];
	int day_count = 0;
	char time_duration[128];
	char *s;

	if (!isdigit(argv[0][0])) {
		vty_out(vty, "%% Invalid format '%s'\n", argv[0]);
		return CMD_ERR_NOTHING_TODO;
	}

	if (strcmp(argv[1], "lan2wan") && strcmp(argv[1], "wan2lan")) {
		vty_out(vty, "%% Not supported option(%s).\n", argv[1]);
		return CMD_WARNING;
	}

	if (strcmp(argv[2], "in") && strcmp(argv[2], "out") && strcmp(argv[2], "fw")) {
		vty_out(vty, "%% Not supported option(%s).\n", argv[2]);
		return CMD_WARNING;
	}

	if (strcmp(argv[3], "insert") && strcmp(argv[3], "append")) {
		vty_out(vty, "%% Not supported option(%s).\n", argv[3]);
		return CMD_WARNING;
	}

	if (strcmp(argv[4], "permit") && strcmp(argv[4], "deny") && strcmp(argv[4], "reject")) {
		vty_out(vty, "%% Not supported option(%s).\n", argv[4]);
		return CMD_WARNING;
	}

	if (!strcmp(argv[7], "any")) {
		sport = 0;
	} else {
		if (!isdigit(argv[7][0])) {
			vty_out(vty, "%% Invalid port number '%s'\n", argv[7]);
			return CMD_ERR_NOTHING_TODO;
		}
		sport = atoi(argv[7]);
		if (sport <= 0 || sport > 65535) {
			vty_out(vty, "%% Invalid port number '%s'\n", argv[7]);
			return CMD_ERR_NOTHING_TODO;
		}
	}

	if (!strcmp(argv[9], "any")) {
		dport = 0;
	} else {
		if (!isdigit(argv[9][0])) {
			vty_out(vty, "%% Invalid port number '%s'\n", argv[9]);
			return CMD_ERR_NOTHING_TODO;
		}
		dport = atoi(argv[9]);
		if (dport <= 0 || dport > 65535) {
			vty_out(vty, "%% Invalid port number '%s'\n", argv[9]);
			return CMD_ERR_NOTHING_TODO;
		}
	}

	/* ip address/netmask sanity check */
	if (!isdigit(argv[6][0])) {
		vty_out(vty, "%% Invalid ip address '%s'\n", argv[6]);
		return CMD_ERR_NOTHING_TODO;
	}
	sprintf(sip, "%s", argv[6]);
	s = strstr(sip, "/");
	if (s) {
		if (strlen(s) > 3) {
			vty_out(vty, "%% Invalid netmask '%s'\n", &s[1]);
			return CMD_ERR_NOTHING_TODO;
		}
	} else {
		vty_out(vty, "%% Invalid ip address/netmask '%s'\n", argv[6]);
		return CMD_ERR_NOTHING_TODO;
	}

	if (!isdigit(argv[8][0])) {
		vty_out(vty, "%% Invalid ip address '%s'\n", argv[8]);
		return CMD_ERR_NOTHING_TODO;
	}
	sprintf(dip, "%s", argv[8]);
	s = strstr(dip, "/");
	if (s) {
		if (strlen(s) > 3) {
			vty_out(vty, "%% Invalid netmask '%s'\n", &s[1]);
			return CMD_ERR_NOTHING_TODO;
		}
	} else {
		vty_out(vty, "%% Invalid ip address/netmask '%s'\n", argv[8]);
		return CMD_ERR_NOTHING_TODO;
	}

	/* Sanity check for day string(mon|tue|wed|thu|fri|sat|sun) */
	if (strcmp(argv[10], "any")) {
		sprintf(day, "%s", argv[10]);
		s = strtok(day, "|");
		while (s) {
			if (strlen(s) > 3 || strlen(s) < 3) {
				vty_out(vty, "%% Invalid day value '%s'\n", argv[10]);
				return CMD_ERR_NOTHING_TODO;
			}
			day_count++;
			s = strtok(NULL, "|");
		}
		if (day_count > 7) {
			vty_out(vty, "%% Invalid day value '%s'\n", argv[10]);
			return CMD_ERR_NOTHING_TODO;
		}
	}

	/* Sanity check for from-time string(08:00) */
	if (strcmp(argv[11], "any")) {
		sprintf(time_duration, "%s", argv[11]);
		s = strtok(time_duration, ":");
		if (s == NULL) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[11]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (strlen(s) > 2 || strlen(s) < 2) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[11]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (atoi(s) > 23 || atoi(s) < 0) {
			vty_out(vty, "%% Invalid time(hour) value '%s'\n", argv[11]);
			return CMD_ERR_NOTHING_TODO;
		}
		s = strtok(NULL, ":");
		if (s == NULL) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[11]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (strlen(s) > 2 || strlen(s) < 2) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[11]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (atoi(s) > 59 || atoi(s) < 0) {
			vty_out(vty, "%% Invalid time(minute) value '%s'\n", argv[11]);
			return CMD_ERR_NOTHING_TODO;
		}
	}

	/* Sanity check for to-time string(21:00) */
	if (strcmp(argv[12], "any")) {
		sprintf(time_duration, "%s", argv[12]);
		s = strtok(time_duration, ":");
		if (s == NULL) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[12]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (strlen(s) > 2 || strlen(s) < 2) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[12]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (atoi(s) > 23 || atoi(s) < 0) {
			vty_out(vty, "%% Invalid time(hour) value '%s'\n", argv[12]);
			return CMD_ERR_NOTHING_TODO;
		}
		s = strtok(NULL, ":");
		if (s == NULL) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[12]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (strlen(s) > 2 || strlen(s) < 2) {
			vty_out(vty, "%% Invalid time value '%s'\n", argv[12]);
			return CMD_ERR_NOTHING_TODO;
		}
		if (atoi(s) > 59 || atoi(s) < 0) {
			vty_out(vty, "%% Invalid time(minute) value '%s'\n", argv[12]);
			return CMD_ERR_NOTHING_TODO;
		}
	}

	sprintf(line, "sfirewall filter %s %s %s %s %s %s %s %s %s %s %s %s %s",
		argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8],
		argv[9], argv[10], argv[11], argv[12]);
	config_del_line(config_top, line);

	ENSURE_CONFIG(vty);

	return 0;
}

DEFUN (sfirewall_filter_rules_apply,
        sfirewall_filter_rules_apply_cmd,
        "sfirewall filter apply",
        "Configure smart firewall rules\n"
		"Add filter rules\n"
        "Apply\n")
{
	char *filter_config_file = CONFIG_DIR "/" "filter.txt";
	char *sfirewall_filter_rule_file = "/etc/config/sfirewall_filter";
	FILE *rfp, *wfp;
	char xbuf[1024], *s;
	char param[13][64]; 
	char *action[3] = { "ACCEPT", "DROP", "REJECT" };
	int action_index = 0;
	int count = 0;
	int filter_rule_len = strlen("sfirewall filter");
	int i;

	wfp = fopen(sfirewall_filter_rule_file, "w");
	if (!wfp)
		return CMD_WARNING;
	fprintf(wfp, "\n");

	rfp = fopen(filter_config_file, "r");
	if (!rfp)
		return CMD_WARNING;

	memset(xbuf, 0, sizeof(xbuf));
	while (fgets(xbuf, sizeof(xbuf), rfp)) {
		xbuf[strlen(xbuf)-1] = '\0';

		// ex: sfirewall filter 10 lan2wan fw insert deny tcp 0.0.0.0/0 any 0.0.0.0/0 80 mon|tue|wed|thu|fri 21:00 09:00
		if (!strncmp(xbuf, "sfirewall filter", filter_rule_len)) {
			s = strtok(&xbuf[filter_rule_len+1], " "); // 10
			if (s == NULL) continue;
			sprintf(param[0], "%s", s);

			s = strtok(NULL, " ");      // lan2wan
			if (s == NULL) continue;
			sprintf(param[1], "%s", s);

			s = strtok(NULL, " ");      // fw
			if (s == NULL) continue;
			sprintf(param[2], "%s", s);

			s = strtok(NULL, " ");      // insert
			if (s == NULL) continue;
			sprintf(param[3], "%s", s);

			s = strtok(NULL, " ");      // deny
			if (s == NULL) continue;
			sprintf(param[4], "%s", s);

			s = strtok(NULL, " ");      // tcp
			if (s == NULL) continue;
			sprintf(param[5], "%s", s);

			s = strtok(NULL, " ");      // 0.0.0.0/0
			if (s == NULL) continue;
			sprintf(param[6], "%s", s);

			s = strtok(NULL, " ");      // any
			if (s == NULL) continue;
			sprintf(param[7], "%s", s);

			s = strtok(NULL, " ");      // 0.0.0.0/0
			if (s == NULL) continue;
			sprintf(param[8], "%s", s);

			s = strtok(NULL, " ");      // 80
			if (s == NULL) continue;
			sprintf(param[9], "%s", s);

			s = strtok(NULL, " ");      // mon|tue|wed|thu|fri
			if (s == NULL) continue;
			sprintf(param[10], "%s", s);
			for (i=0; i<strlen(param[10]); i++) {
				if (param[10][i] == '|')
					param[10][i] = ' ';
			}

			s = strtok(NULL, " ");      // 21:00
			if (s == NULL) continue;
			sprintf(param[11], "%s", s);

			s = strtok(NULL, " ");      // 09:00
			if (s == NULL) continue;
			sprintf(param[12], "%s", s);
		
			// ---------------------------------------------
			
			if (!strcmp(param[4], "permit"))
				action_index = 0;
			else if (!strcmp(param[4], "deny"))
				action_index = 1;
			else if (!strcmp(param[4], "reject"))
				action_index = 2;

			fprintf(wfp, "config rule\n");
			if (!strcmp(param[1], "lan2wan")) {
				fprintf(wfp, "        option src 'lan'\n");
				fprintf(wfp, "        option dest 'wan'\n");
			} else if (!strcmp(param[1], "wan2lan")) {
				fprintf(wfp, "        option src 'wan'\n");
				fprintf(wfp, "        option dest 'lan'\n");
			}
			fprintf(wfp, "        option proto '%s'\n", param[5]);
			fprintf(wfp, "        option family 'ipv4'\n");
			fprintf(wfp, "        option src_ip '%s'\n", param[6]);
			if (strcmp(param[7], "any"))
				fprintf(wfp, "        option src_port '%s'\n", param[7]);
			fprintf(wfp, "        option dest_ip '%s'\n", param[8]);
			if (strcmp(param[9], "any"))
				fprintf(wfp, "        option dest_port '%s'\n", param[9]);
			if (strcmp(param[10], "any"))
				fprintf(wfp, "        option weekdays '%s'\n", param[10]);
			if (strcmp(param[11], "any")) {
				fprintf(wfp, "        option start_time '%s'\n", param[11]);
				fprintf(wfp, "        option stop_time '%s'\n", param[12]);
			}
			fprintf(wfp, "        option target '%s'\n", action[action_index]);
			fprintf(wfp, "        option name 'SFIREWALL_FILTER%d'\n", count++);
			fprintf(wfp, "        option enabled '1'\n");
			fprintf(wfp, "\n");
		}
		memset(xbuf, 0, sizeof(xbuf));
	}
	fclose(wfp);
	fclose(rfp);

	apply_firewall_rules();

    return CMD_SUCCESS;
}

#if 0
DEFUN (sfirewall_policy_youtubefilter,
		sfirewall_policy_youtubefilter_cmd,
		"sfirewall policy youtubefilter",
		"Configure smart firewall rules\n"
		"Set policy rules\n"
		"Enable YouTube DNS filter\n")
{
	char szInfo[1024];
	FILE *wfp;

	sprintf(szInfo, "sfirewall policy youtubefilter");
	config_del_line(config_top, szInfo);
	config_add_line(config_top, szInfo);

	ENSURE_CONFIG(vty);

	wfp = fopen("/etc/hosts", "w");
	if (!wfp)
		return CMD_WARNING;

	fprintf(wfp, "127.0.0.1 localhost\n");
	fprintf(wfp, "\n");
	fprintf(wfp, "::1     localhost ip6-localhost ip6-loopback\n");
	fprintf(wfp, "ff02::1 ip6-allnodes\n");
	fprintf(wfp, "ff02::2 ip6-allrouters\n");
	fprintf(wfp, "\n");
	fprintf(wfp, "#youtube filtering\n");
	fprintf(wfp, "127.0.0.1 www.youtube.com\n");
	fprintf(wfp, "127.0.0.1 youtube.com\n");

	fclose(wfp);

	return CMD_SUCCESS;
}

DEFUN (no_sfirewall_policy_youtubefilter,
		no_sfirewall_policy_youtubefilter_cmd,
		"no sfirewall policy youtubefilter",
		NO_STR
		"Configure smart firewall rules\n"
		"Set policy rules\n"
		"Enable YouTube DNS filter\n")
{
	char szInfo[1024];
	FILE *wfp;

	sprintf(szInfo, "sfirewall policy youtubefilter");
	config_del_line(config_top, szInfo);

	ENSURE_CONFIG(vty);

	wfp = fopen("/etc/hosts", "w");
	if (!wfp)
		return CMD_WARNING;

	fprintf(wfp, "127.0.0.1 localhost\n");
	fprintf(wfp, "\n");
	fprintf(wfp, "::1     localhost ip6-localhost ip6-loopback\n");
	fprintf(wfp, "ff02::1 ip6-allnodes\n");
	fprintf(wfp, "ff02::2 ip6-allrouters\n");
	fprintf(wfp, "\n");

	fclose(wfp);

	return CMD_SUCCESS;
}
#endif

DEFUN (show_sfirewall,
		show_sfirewall_cmd,
		"show sfirewall (all|nat|filter|mangle)",
		SHOW_STR
		"show smart firewall rules\n"
		"show all rules\n"
		"show NAT rules\n"
		"show filter rules\n"
		"show mangle rules\n")
{
	if (!strcmp(argv[0], "all"))
		system("iptables -n -v -L");
	else if (!strcmp(argv[0], "nat"))
		system("iptables -t nat -n -v -L");
	else if (!strcmp(argv[0], "filter"))
		system("iptables -t filter -n -v -L");
	else if (!strcmp(argv[0], "mangle"))
		system("iptables -t mangle -n -v -L");
	return CMD_SUCCESS;
}

int cmd_firewall_init()
{
	// enable/disable
#if 0
	cmd_install_element (CONFIG_NODE, &sfirewall_enable_cmd);
	cmd_install_element (CONFIG_NODE, &no_sfirewall_enable_cmd);
#endif

	// portmap
	cmd_install_element (CONFIG_NODE, &sfirewall_nat_portmap_cmd);
	cmd_install_element (CONFIG_NODE, &no_sfirewall_nat_portmap_cmd);
	cmd_install_element (CONFIG_NODE, &sfirewall_nat_portmap_apply_cmd);

	// filter
	cmd_install_element (CONFIG_NODE, &sfirewall_filter_rules_cmd);
	cmd_install_element (CONFIG_NODE, &no_sfirewall_filter_rules_cmd);
	cmd_install_element (CONFIG_NODE, &sfirewall_filter_rules_apply_cmd);

#if 0
	cmd_install_element (CONFIG_NODE, &sfirewall_policy_youtubefilter_cmd);
	cmd_install_element (CONFIG_NODE, &no_sfirewall_policy_youtubefilter_cmd);
#endif

	// show
	cmd_install_element (ENABLE_NODE, &show_sfirewall_cmd);
	cmd_install_element (CONFIG_NODE, &show_sfirewall_cmd);

	return 0;
}

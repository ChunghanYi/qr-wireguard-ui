/*
 * WireGuard VPN
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
#include "../encoding.h"

/*
 * wg listenport PORT
 * wg peer PUBLICKEY allowed-ips (none|WORD) endpoint (none|FQDN:PORT|A.B.C.D:PORT) persistent-keepalive (off|NUM)
 * no wg peer PUBLIC KEY
 * wg link-up|link-down
 * wg regenerate-key
 * wg clear-key
 * show wg
 * show wg ETHNAME
 */

#define PRIVATEKEY_PATH     CONFIG_DIR "/privatekey"
#define PUBLICKEY_PATH      CONFIG_DIR "/publickey"
#define PUBLICKEY_MAX_LEN   44

///////////////////////////////////////////////////////////////////////////////////////

DEFUN (wg_listenport,
		wg_listenport_cmd,
		"wg listenport NUM",
		"Configure WireGuard rules\n"
		"Listening port\n"
		"1024 ~ 65535\n")
{
	int nNum = atoi(argv[0]);
	char szInfo[1024], xbuf[256];
	FILE *fp = NULL;
	int fwrule = -1;

	snprintf(szInfo, sizeof(szInfo), "wg listenport");
    config_del_line_byleft(config_top, szInfo);
    snprintf(szInfo, sizeof(szInfo), "wg listenport %d", nNum);
    config_add_line(config_top, szInfo);

    ENSURE_CONFIG(vty);

	/* wg set wg0 listen-port PORT */
	sprintf(szInfo, "wg set wg0 listen-port %s > /dev/null 2>&1", argv[0]);
	system(szInfo);

	/* Delete the existing UCI firewall rule for WG listen port */
	/*
	 * fwrule=`uci show firewall  | grep "Allow-WG-Inbound" | awk -F'=' '{ print $1 }' | sed s"/.name//g"`
	 * uci delete $fwrule > /dev/null 2>&1
	 * uci commit firewall > /dev/null 2>&1
	 */
	system("uci show firewall  | grep \"Allow-WG-Inbound\" | awk -F'=' '{ print $1 }' | sed s\"/.name//g\" > /tmp/.fwrule");
	fp = fopen("/tmp/.fwrule", "r");
	if (fp) {
		memset(xbuf, 0, sizeof(xbuf));
		if (fgets(xbuf, sizeof(xbuf), fp)) {
			fwrule = atoi(xbuf);
		}
		fclose(fp);
	}
	unlink("/tmp/.fwrule");
	if (fwrule > 0) {
		sprintf(szInfo, "uci delete %d > /dev/null 2>&1", fwrule);
		system(szInfo);
	}

	/* Add a new UCI firewall rule for WG listen port */
	system("uci add firewall rule > /dev/null 2>&1");
	system("uci set firewall.@rule[-1].src=\"*\" > /dev/null 2>&1");
	system("uci set firewall.@rule[-1].target=\"ACCEPT\" > /dev/null 2>&1");
	system("uci set firewall.@rule[-1].proto=\"udp\" > /dev/null 2>&1");
	sprintf(szInfo, "uci set firewall.@rule[-1].dest_port=\"%s\" > /dev/null 2>&1", argv[0]);
	system(szInfo);
	system("uci set firewall.@rule[-1].name=\"Allow-WG-Inbound\" > /dev/null 2>&1");
	system("uci commit firewall > /dev/null 2>&1");
	system("/etc/init.d/firewall restart > /dev/null 2>&1");

	return CMD_SUCCESS;
}

/* debug code */
#if 0
DEFUN (wg_peer_full,
		wg_peer_full_cmd,
		"wg peer PUBLICKEY allowed-ips (none|WORD) endpoint (none|FQDN:PORT|A.B.C.D:PORT) persistent-keepalive (off|NUM)",
		"Configure WireGuard rules\n"
		"Specify peer information\n"
		"Public key\n"
		"Allow ip addresses\n"
		"no ip network\n"
		"ip network e.g. 192.168.1.0/24,172.16.0.0/16\n"
		"Endpoint information\n"
		"no ip address and port\n"
		"FQDN and port e.g. test.yourdomain.com:12345\n"
		"ip address and port e.g. x.x.x.x:y\n"
		"Set persistent-keepalive time\n"
		"no keepalive\n"
		"Seconds 1-1800\n")
{
	char szInfo[2048];
	struct stat sb;
	int knum;

	/* sanity check for public key ! */
	if (strlen(argv[0]) != PUBLICKEY_MAX_LEN || argv[0][strlen(argv[0])-1] != '=') { /* public key size */
		vty_out (vty, "%% Public key length mismatch %s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
	}

	if (strcmp(argv[1], "none") && !isdigit(argv[1][0])) {
		vty_out (vty, "%% Invalid allowed-ips '%s', Please input the correct value %s",
			argv[1], VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
	}

	if (strlen(argv[2]) > 128) {
		vty_out (vty, "%% Invalid endpoint '%s', Too long FQDN %s",
				argv[2], VTY_NEWLINE);
		return CMD_ERR_NOTHING_TODO;
	}

	if (strchr(argv[2], '/')) {
		vty_out(vty, "%% Invalid endpoint '%s', endpoint can't include slash.\n", argv[2]);
		return CMD_WARNING;
	}

	if (isdigit(argv[3][0])) {
		knum = atoi(argv[3]);
		if (knum < 1 || knum > 1800) {
			vty_out (vty, "%% Invalid persistent-keepalive time '%s', Please input the correct value %s",
					argv[3], VTY_NEWLINE);
			return CMD_ERR_NOTHING_TODO;
		}
	}

	if (stat(PRIVATEKEY_PATH, &sb) == -1) {
		sprintf(szInfo, "wg genkey | tee %s/privatekey | wg pubkey > %s/publickey",
			CONFIG_DIR, CONFIG_DIR);
		system(szInfo);
	}

	snprintf(szInfo, sizeof(szInfo), "wg peer %s", argv[0]);
    config_del_line_byleft(config_top, szInfo);

	snprintf(szInfo, sizeof(szInfo), "wg peer %s allowed-ips %s endpoint %s persistent-keepalive %s",
		argv[0], argv[1], argv[2], argv[3]);
    config_add_line(config_top, szInfo);

    ENSURE_CONFIG(vty);

	/*
	 * wg set wg0 private-key privatekey peer SERVERPUB allowed-ips 5.5.5.0/24 \
	 * endpoint vpn.server.com:12000 persistent-keepalive 10
 	 */

	/* 00 */
	if (strcmp(argv[1], "none") && strcmp(argv[2], "none")) {
		sprintf(szInfo, "wg set wg0 private-key %s peer %s allowed-ips %s endpoint %s persistent-keepalive %s > /dev/null 2>&1",
			PRIVATEKEY_PATH,
			argv[0], argv[1], argv[2], argv[3]);

	/* 01 */
	} else if (strcmp(argv[1], "none") && !strcmp(argv[2], "none")) {
		sprintf(szInfo, "wg set wg0 private-key %s peer %s allowed-ips %s persistent-keepalive %s > /dev/null 2>&1",
			PRIVATEKEY_PATH,
			argv[0], argv[1], argv[3]);

	/* 10 */	
	} else if (!strcmp(argv[1], "none") && strcmp(argv[2], "none")) {
		sprintf(szInfo, "wg set wg0 private-key %s peer %s endpoint %s persistent-keepalive %s > /dev/null 2>&1",
			PRIVATEKEY_PATH,
			argv[0], argv[2], argv[3]);

	/* 11 */
	} else if (!strcmp(argv[1], "none") && !strcmp(argv[2], "none")) {
		sprintf(szInfo, "wg set wg0 private-key %s peer %s persistent-keepalive %s > /dev/null 2>&1",
			PRIVATEKEY_PATH,
			argv[0], argv[3]);
	}
	system(szInfo);

	return CMD_SUCCESS;
}
#endif

DEFUN (wg_peer_public_key,
		wg_peer_public_key_cmd,
		"wg peer PUBLICKEY",
		"Configure WireGuard rules\n"
		"Specify peer information\n"
		"Public key\n")
{
	char szInfo[2048];
	struct stat sb;

	/* sanity check for public key ! */
	if (strlen(argv[0]) != PUBLICKEY_MAX_LEN || argv[0][strlen(argv[0])-1] != '=') { /* public key size */
		vty_out (vty, "%% Public key length mismatch %s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
	}

	if (stat(PRIVATEKEY_PATH, &sb) == -1) {
		sprintf(szInfo, "wg genkey | tee %s/privatekey | wg pubkey > %s/publickey",
			CONFIG_DIR, CONFIG_DIR);
		system(szInfo);
	}

	snprintf(szInfo, sizeof(szInfo), "wg peer %s", argv[0]);
    config_del_line_byleft(config_top, szInfo);

	snprintf(szInfo, sizeof(szInfo), "wg peer %s", argv[0]);
    config_add_line(config_top, szInfo);

    ENSURE_CONFIG(vty);

	/*
	 * wg set wg0 private-key privatekey peer SERVERPUB allowed-ips 5.5.5.0/24 \
	 * endpoint vpn.server.com:12000 persistent-keepalive 10
 	 */
	sprintf(szInfo, "wg set wg0 private-key %s peer %s > /dev/null 2>&1",
			PRIVATEKEY_PATH, argv[0]);
	system(szInfo);

	return CMD_SUCCESS;
}

DEFUN (wg_peer_allowed_ips,
		wg_peer_allowed_ips_cmd,
		"wg peer PUBLICKEY allowed-ips WORD",
		"Configure WireGuard rules\n"
		"Specify peer information\n"
		"Public key\n"
		"Allow ip addresses\n"
		"ip network e.g. 192.168.1.0/24,172.16.0.0/16\n")
{
	char szInfo[2048];
	struct stat sb;

	/* sanity check for public key ! */
	if (strlen(argv[0]) != PUBLICKEY_MAX_LEN || argv[0][strlen(argv[0])-1] != '=') { /* public key size */
		vty_out (vty, "%% Public key length mismatch %s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
	}

	if (!isdigit(argv[1][0])) {
		vty_out (vty, "%% Invalid allowed-ips '%s', Please input the correct value %s",
			argv[1], VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
	}

	if (stat(PRIVATEKEY_PATH, &sb) == -1) {
		sprintf(szInfo, "wg genkey | tee %s/privatekey | wg pubkey > %s/publickey",
			CONFIG_DIR, CONFIG_DIR);
		system(szInfo);
	}

	snprintf(szInfo, sizeof(szInfo), "wg peer %s", argv[0]);
    config_del_line_byleft(config_top, szInfo);

	snprintf(szInfo, sizeof(szInfo), "wg peer %s allowed-ips %s", argv[0], argv[1]);
    config_add_line(config_top, szInfo);

    ENSURE_CONFIG(vty);

	/*
	 * wg set wg0 private-key privatekey peer SERVERPUB allowed-ips 5.5.5.0/24
 	 */
	sprintf(szInfo, "wg set wg0 private-key %s peer %s allowed-ips %s > /dev/null 2>&1",
			PRIVATEKEY_PATH, argv[0], argv[1]);
	system(szInfo);

	return CMD_SUCCESS;
}

DEFUN (wg_peer_endpoint,
		wg_peer_endpoint_cmd,
		"wg peer PUBLICKEY allowed-ips WORD endpoint (none|FQDN:PORT|A.B.C.D:PORT)",
		"Configure WireGuard rules\n"
		"Specify peer information\n"
		"Public key\n"
		"Allow ip addresses\n"
		"ip network e.g. 192.168.1.0/24,172.16.0.0/16\n"
		"Endpoint information\n"
		"no ip address and port\n"
		"FQDN and port e.g. test.yourdomain.com:12345\n"
		"ip address and port e.g. x.x.x.x:y\n")
{
	char szInfo[2048];
	struct stat sb;

	/* sanity check for public key ! */
	if (strlen(argv[0]) != PUBLICKEY_MAX_LEN || argv[0][strlen(argv[0])-1] != '=') { /* public key size */
		vty_out (vty, "%% Public key length mismatch %s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
	}

	if (!isdigit(argv[1][0])) {
		vty_out (vty, "%% Invalid allowed-ips '%s', Please input the correct value %s",
			argv[1], VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
	}

	if (strlen(argv[2]) > 128) {
		vty_out (vty, "%% Invalid endpoint '%s', Too long FQDN %s",
				argv[2], VTY_NEWLINE);
		return CMD_ERR_NOTHING_TODO;
	}

	if (strchr(argv[2], '/')) {
		vty_out(vty, "%% Invalid endpoint '%s', endpoint can't include slash.\n", argv[2]);
		return CMD_WARNING;
	}

	if (stat(PRIVATEKEY_PATH, &sb) == -1) {
		sprintf(szInfo, "wg genkey | tee %s/privatekey | wg pubkey > %s/publickey",
			CONFIG_DIR, CONFIG_DIR);
		system(szInfo);
	}

	snprintf(szInfo, sizeof(szInfo), "wg peer %s", argv[0]);
    config_del_line_byleft(config_top, szInfo);

	snprintf(szInfo, sizeof(szInfo), "wg peer %s allowed-ips %s endpoint %s",
		argv[0], argv[1], argv[2]);
    config_add_line(config_top, szInfo);

    ENSURE_CONFIG(vty);

	/*
	 * wg set wg0 private-key privatekey peer SERVERPUB allowed-ips 5.5.5.0/24 \
	 * endpoint vpn.server.com:12000
 	 */
	sprintf(szInfo, "wg set wg0 private-key %s peer %s allowed-ips %s endpoint %s > /dev/null 2>&1",
			PRIVATEKEY_PATH, argv[0], argv[1], argv[2]);
	system(szInfo);

	return CMD_SUCCESS;
}

DEFUN (wg_peer_persistent_keepalive,
		wg_peer_persistent_keepalive_cmd,
		"wg peer PUBLICKEY allowed-ips WORD endpoint (none|FQDN:PORT|A.B.C.D:PORT) persistent-keepalive (off|NUM)",
		"Configure WireGuard rules\n"
		"Specify peer information\n"
		"Public key\n"
		"Allow ip addresses\n"
		"ip network e.g. 192.168.1.0/24,172.16.0.0/16\n"
		"Endpoint information\n"
		"no ip address and port\n"
		"FQDN and port e.g. test.yourdomain.com:12345\n"
		"ip address and port e.g. x.x.x.x:y\n"
		"Set persistent-keepalive time\n"
		"no keepalive\n"
		"Seconds 1-1800\n")
{
	char szInfo[2048];
	struct stat sb;
	int knum;

	/* sanity check for public key ! */
	if (strlen(argv[0]) != PUBLICKEY_MAX_LEN || argv[0][strlen(argv[0])-1] != '=') { /* public key size */
		vty_out (vty, "%% Public key length mismatch %s", VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
	}

	if (!isdigit(argv[1][0])) {
		vty_out (vty, "%% Invalid allowed-ips '%s', Please input the correct value %s",
			argv[1], VTY_NEWLINE);
        return CMD_ERR_NOTHING_TODO;
	}

	if (strlen(argv[2]) > 128) {
		vty_out (vty, "%% Invalid endpoint '%s', Too long FQDN %s",
				argv[2], VTY_NEWLINE);
		return CMD_ERR_NOTHING_TODO;
	}

	if (strchr(argv[2], '/')) {
		vty_out(vty, "%% Invalid endpoint '%s', endpoint can't include slash.\n", argv[2]);
		return CMD_WARNING;
	}
	
	if (isdigit(argv[3][0])) {
		knum = atoi(argv[3]);
		if (knum < 1 || knum > 1800) {
			vty_out (vty, "%% Invalid persistent-keepalive time '%s', Please input the correct value %s",
					argv[3], VTY_NEWLINE);
			return CMD_ERR_NOTHING_TODO;
		}
	}

	if (stat(PRIVATEKEY_PATH, &sb) == -1) {
		sprintf(szInfo, "wg genkey | tee %s/privatekey | wg pubkey > %s/publickey",
			CONFIG_DIR, CONFIG_DIR);
		system(szInfo);
	}

	snprintf(szInfo, sizeof(szInfo), "wg peer %s", argv[0]);
    config_del_line_byleft(config_top, szInfo);

	snprintf(szInfo, sizeof(szInfo), "wg peer %s allowed-ips %s endpoint %s persistent-keepalive %s",
		argv[0], argv[1], argv[2], argv[3]);
    config_add_line(config_top, szInfo);

    ENSURE_CONFIG(vty);

	/*
	 * wg set wg0 private-key privatekey peer SERVERPUB allowed-ips 5.5.5.0/24 \
	 * endpoint vpn.server.com:12000 persistent-keepalive 10
 	 */
	sprintf(szInfo, "wg set wg0 private-key %s peer %s allowed-ips %s endpoint %s persistent-keepalive %s > /dev/null 2>&1",
			PRIVATEKEY_PATH, argv[0], argv[1], argv[2], argv[3]);
	system(szInfo);

	return CMD_SUCCESS;
}

DEFUN (no_wg_peer,
		no_wg_peer_cmd,
		"no wg peer PUBLICKEY",
		NO_STR
		"Configure WireGuard rules\n"
		"Specify peer information\n"
		"Public key\n")
{
	char szInfo[1024];

	snprintf(szInfo, sizeof(szInfo), "wg peer %s", argv[0]);
    config_del_line_byleft(config_top, szInfo);

    ENSURE_CONFIG(vty);

	sprintf(szInfo, "wg set wg0 peer %s remove > /dev/null 2>&1", argv[0]);
	system(szInfo);

	return CMD_SUCCESS;
}

DEFUN (wg_link,
		wg_link_cmd,
		"wg (link-up|link-down)",
		"Configure WireGuard rules\n"
		"Enable wireguard link\n"
		"Disable wireguard link\n")
{
	char szInfo[1024];

	if (strcmp(argv[0], "link-up") == 0) {
		snprintf(szInfo, sizeof(szInfo), "wg link-up");
		config_del_line_byleft(config_top, szInfo);

		snprintf(szInfo, sizeof(szInfo), "wg link-down");
		config_del_line_byleft(config_top, szInfo);

		snprintf(szInfo, sizeof(szInfo), "wg link-up");
		config_add_line(config_top, szInfo);

		ENSURE_CONFIG(vty);

		sprintf(szInfo, "ip link set up dev wg0 > /dev/null 2>&1");
		system(szInfo);
	} else {
		snprintf(szInfo, sizeof(szInfo), "wg link-up");
		config_del_line_byleft(config_top, szInfo);

		snprintf(szInfo, sizeof(szInfo), "wg link-down");
		config_del_line_byleft(config_top, szInfo);

		snprintf(szInfo, sizeof(szInfo), "wg link-down");
		config_add_line(config_top, szInfo);

		ENSURE_CONFIG(vty);

		sprintf(szInfo, "ip link set down dev wg0 > /dev/null 2>&1");
		system(szInfo);
	}

	return CMD_SUCCESS;
}

DEFUN (wg_rekey,
		wg_rekey_cmd,
		"wg regenerate-key",
		"Configure WireGuard rules\n"
		"Regenerate private & public keys\n")
{
	char szInfo[1024];
	FILE *fp = NULL;
	char xbuf[1024];

	sprintf(szInfo, "wg genkey | tee %s/privatekey | wg pubkey > %s/publickey",
			CONFIG_DIR, CONFIG_DIR);
	system(szInfo);

	sprintf(szInfo, "wg set wg0 private-key %s/privatekey", CONFIG_DIR);
	system(szInfo);

	vty_out (vty, "My Private key => [hidden]\n");
	fp = fopen(PUBLICKEY_PATH, "r");
	if (fp) {
		memset(xbuf, 0, sizeof(xbuf));
		if (fgets(xbuf, sizeof(xbuf), fp)) {
			vty_out (vty, "My Public key => %s", xbuf);
		}
		fclose(fp);
	}

	return CMD_SUCCESS;
}

DEFUN (wg_clearkey,
		wg_clearkey_cmd,
		"wg clear-key",
		"Configure WireGuard rules\n"
		"Remove curve25519 private & public keys\n")
{
	char szInfo[1024];

	sprintf(szInfo, "rm %s/publickey > /dev/null 2>&1", CONFIG_DIR);
	system(szInfo);
	sprintf(szInfo, "rm %s/privatekey > /dev/null 2>&1", CONFIG_DIR);
	system(szInfo);

	return CMD_SUCCESS;
}

DEFUN (show_wg,
		show_wg_cmd,
		"show wg",
		SHOW_STR
		"Show the wireguard tunnel info\n")
{
	system("wg show");
	return CMD_SUCCESS;
}

DEFUN (show_wg_conf,
		show_wg_conf_cmd,
		"show wg ETHNAME",
		SHOW_STR
		"Show the wireguard tunnel info\n"
		"Show the tunnel info for the specified interface\n")
{
	char szInfo[1024];

	sprintf(szInfo, "wg showconf %s", argv[0]);
	system(szInfo);

	return CMD_SUCCESS;
}

int cmd_vpn_init()
{
	cmd_install_element (ENABLE_NODE, &show_wg_cmd);
	cmd_install_element (CONFIG_NODE, &show_wg_cmd);
	cmd_install_element (ENABLE_NODE, &show_wg_conf_cmd);
	cmd_install_element (CONFIG_NODE, &show_wg_conf_cmd);

	cmd_install_element (CONFIG_NODE, &wg_listenport_cmd);
	cmd_install_element (CONFIG_NODE, &wg_peer_public_key_cmd);
	cmd_install_element (CONFIG_NODE, &wg_peer_allowed_ips_cmd);
	cmd_install_element (CONFIG_NODE, &wg_peer_endpoint_cmd);
	cmd_install_element (CONFIG_NODE, &wg_peer_persistent_keepalive_cmd);
	cmd_install_element (CONFIG_NODE, &no_wg_peer_cmd);

	cmd_install_element (CONFIG_NODE, &wg_link_cmd);
	cmd_install_element (CONFIG_NODE, &wg_rekey_cmd);
	cmd_install_element (CONFIG_NODE, &wg_clearkey_cmd);

	return 0;
}

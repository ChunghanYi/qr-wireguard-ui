/*
 * Tools
 * Copyright (c) 2024 Chunghan Yi <chunghan.yi@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "command.h"
#include "vtysh_config.h"
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>

#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>

DEFUN(config_sh,
        config_sh_cmd,
        "end WORD",
        "End a cli\n"
        "End a cli\n")
{
	if (strcmp(argv[0], "shell")) {
		vty_out (vty, "%% Invalid magic value.%s", VTY_NEWLINE);
		return CMD_ERR_NOTHING_TODO;
	}

	system("/bin/ash");
	return CMD_SUCCESS;
}

DEFUN(config_ping,
        config_ping_cmd,
        "ping WORD",
        "Send echo messages\n"
        "Ping destination address or hostname\n")
{
	cmd_execute_system_command ("ping", 1, argv);
	return CMD_SUCCESS;
}

DEFUN(config_ping_count,
        config_ping_cmd_count,
        "ping WORD NUM",
        "send count echo messages\n"
        "Ping destination address or hostname\n"
        "Stop after sending NUM ECHO_REQUEST packets\n")
{
	char *myargv[5];

	if (!isdigit(argv[1][0])) {
		vty_out(vty, "%% Invalid number '%s', Please input the count number\n", argv[1]);
		return CMD_ERR_NOTHING_TODO;
	}
	myargv[0] = argv[0];
	myargv[1] = "-c";
	myargv[2] = argv[1];
	cmd_execute_system_command ("ping", 3, myargv);
	return CMD_SUCCESS;
}

DEFUN(config_ssh,
        config_ssh_cmd,
        "ssh WORD",
        "Open a ssh connection\n"
        "UserID@IP address or hostname of a remote system\n")
{
	cmd_execute_system_command ("ssh", 1, argv);
	return CMD_SUCCESS;
}

DEFUN(config_ssh_port,
        config_ssh_port_cmd,
        "ssh WORD PORT",
        "Open a ssh connection\n"
        "UserID@IP address or hostname of a remote system\n"
        "SSH Port number\n")
{
	char *myargv[5];

	myargv[0] = argv[0];
	myargv[1] = "-p";
	myargv[2] = argv[1];
	cmd_execute_system_command ("ssh", 3, myargv);
	return CMD_SUCCESS;
}

DEFUN(config_reboot,
        config_reboot_cmd,
        "reboot",
        "Reboot the system\n"
        "Reboot the system\n")
{
	char xbuf[1024] = {0, };

	vty_out(vty, "Do you really want to reboot the system? (y/n):\n");   /* '\n': libreadline issue */
	if (fgets(xbuf, sizeof(xbuf), stdin)) {
		if (xbuf[0] == 'y') {
			vty_out(vty, "System will be rebooted, please waiting...\n");
			system("sync; sync; sleep 3; reboot -f &");
			exit(1);
		}
	}

	return CMD_SUCCESS;
}

DEFUN(config_poweroff,
        config_poweroff_cmd,
        "poweroff",
        "Power off the system\n"
        "Power off the system\n")
{
	char xbuf[1024] = {0, };

	vty_out(vty, "Do you really want to power off the system? (y/n):\n");  /* '\n': libreadline issue */
	if (fgets(xbuf, sizeof(xbuf), stdin)) {
		if (xbuf[0] == 'y') {
			vty_out(vty, "System will be shutdowned after 3 seconds...\n");
			system("sync; sync; sleep 3; poweroff&");
		}
	}
	return CMD_SUCCESS;
}

DEFUN(config_sysinfo,
        config_sysinfo_cmd,
        "show sysinfo",
        SHOW_STR
        "display the system info\n")
{
	vty_out(vty, "please waiting ...\n");
	system("top -n 1 | head -n 10");
	return 0;
}

DEFUN (config_tcpdump,
        config_tcpdump_cmd,
        "tcpdump INTERFACE EXPRESSION",
        "Tcpdump packet \n"
        "the interface name\n"
        "expression, example:\"port 80 and host 192.168.0.1\"")
{
    char *myargv[10];
    int i;

    for (i = 0; i < strlen(argv[1]); i ++) {
        if (argv[1][i] == '-')
            argv[1][i] = ' ';
    }

    myargv[0] = "-i";
    myargv[1] = argv[0];
    myargv[2] = "-n";
    myargv[3] = "-Z";
    myargv[4] = RUN_USER;
    myargv[5] = "-XX";
    myargv[6] = argv[1];
    return cmd_execute_system_command ("tcpdump", 7, myargv);
}

#if 0
DEFUN(config_date,
        config_date_cmd,
        "date WORD WORD",
        "Set the date\n"
        "the date YYYY-MM-DD\n"
        "the time HH:MM:SS"
     )
{	
	char *myargv[10];
	char date[256];

	myargv[0] = "-s";
	sprintf(date, "%s %s", argv[0], argv[1]);
	myargv[1] = date;
	cmd_execute_system_command("date", 2, myargv);
	return CMD_SUCCESS;
}
#endif

/* Show date. */
DEFUN (show_date,
        show_date_cmd,
        "show date",
        SHOW_STR
        "Displays the current date\n")
{
    return cmd_execute_system_command("date", 0, argv);
}

DEFUN(net_netstat_info,
        net_netstat_info_cmd,
        "netstat",
        "show netstat\n"
        "Params Ctrl+C stop\n")
{ 
	char *myargv[10];

	myargv[0] = "-n";
	myargv[1] = "-a";
	return cmd_execute_system_command ("netstat", 2, myargv);
} 

int cmd_tool_init()
{
	/* Each node's basic commands. */
	cmd_install_element (VIEW_NODE, &config_ping_cmd);
	cmd_install_element (VIEW_NODE, &config_ping_cmd_count);
	cmd_install_element (VIEW_NODE, &config_ssh_cmd);
	cmd_install_element (VIEW_NODE, &config_ssh_port_cmd);

	cmd_install_element (ENABLE_NODE, &config_ping_cmd);
	cmd_install_element (ENABLE_NODE, &config_ping_cmd_count);
	cmd_install_element (ENABLE_NODE, &config_ssh_cmd);
	cmd_install_element (ENABLE_NODE, &config_ssh_port_cmd);
	cmd_install_element (ENABLE_NODE, &config_tcpdump_cmd);
	cmd_install_element (ENABLE_NODE, &net_netstat_info_cmd);
	cmd_install_element (ENABLE_NODE, &config_sysinfo_cmd);
	cmd_install_element (ENABLE_NODE, &show_date_cmd);

	cmd_install_element (CONFIG_NODE, &config_ping_cmd);
	cmd_install_element (CONFIG_NODE, &config_ping_cmd_count);

	cmd_install_element (CONFIG_NODE, &config_reboot_cmd);
	cmd_install_element (CONFIG_NODE, &config_poweroff_cmd);

	cmd_install_element (CONFIG_NODE, &config_ssh_cmd);
	cmd_install_element (CONFIG_NODE, &config_ssh_port_cmd);

#if 0
	cmd_install_element (CONFIG_NODE, &config_date_cmd);
#endif

	cmd_install_element (CONFIG_NODE, &show_date_cmd);
	cmd_install_element (CONFIG_NODE, &config_sh_cmd);

	return 0;
}

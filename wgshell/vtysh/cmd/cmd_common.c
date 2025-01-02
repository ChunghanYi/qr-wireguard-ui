/*
 * Common interfaces
 * Copyright (c) 2024 Chunghan Yi <chunghan.yi@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "command.h"
#include "vtysh_config.h"
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>

#if 0
static char *zencrypt(char *pass)
{
	char *crypt(const char *key, const char *salt);
	char salt[6];

	memset(salt, 0, sizeof(salt));
	memcpy(salt, pass, 2);
	return crypt(pass, salt);
}
#endif

/* Configration from terminal */
DEFUN (config_terminal,
       config_terminal_cmd,
       "configure terminal",
       "Configuration from vty interface\n"
       "Configuration terminal\n")
{
	vty->node = CONFIG_NODE;
	return CMD_SUCCESS;
}

/* Enable command */
DEFUN (enable, 
       config_enable_cmd,
       "enable",
       "Turn on privileged mode command\n")
{
	/* If enable password is NULL, change to ENABLE_NODE */
	if ((host.enable == NULL && host.enable_encrypt == NULL) ||
			vty->type == VTY_FILE)
		vty->node = ENABLE_NODE;
	else
		vty->node = AUTH_ENABLE_NODE;

	return CMD_SUCCESS;
}

/* Disable command */
DEFUN (disable, 
       config_disable_cmd,
       "disable",
       "Turn off privileged mode command\n")
{
	if (vty->node == ENABLE_NODE)
		vty->node = VIEW_NODE;
	return CMD_SUCCESS;
}

/* Down vty node level. */
DEFUN (config_exit,
       config_exit_cmd,
       "exit",
       "Exit current mode and down to previous mode\n")
{
	switch (vty->node) {
		case VIEW_NODE:
			exit (0);
			break;
		case ENABLE_NODE:
			vty->node = VIEW_NODE;
			break;
		case CONFIG_NODE:
			vty->node = ENABLE_NODE;
			break;
		default:
			break;
	}
	return CMD_SUCCESS;
}

#if 0
/* End of configuration. */
DEFUN (config_end,
       config_end_cmd,
       "end",
       "End current mode and change to enable mode.")
{
	switch (vty->node) {
		case VIEW_NODE:
		case ENABLE_NODE:
			/* Nothing to do. */
			break;
		case CONFIG_NODE:
			vty->node = ENABLE_NODE;
			break;
		default:
			break;
	}
	return CMD_SUCCESS;
}
#endif

/* Show version. */
DEFUN (show_version,
		show_version_cmd,
		"show version",
		SHOW_STR
		"Displays the version information\n")
{
	char xbuf[1024];

	memset(xbuf, 0, sizeof(xbuf));
	sprintf(xbuf, "0.09.00");
	vty_out (vty, "Version: %s\n", xbuf);
	vty_out (vty, "Copyright: Chunghan Yi <chunghan.yi@gmail.com>\n");
	vty_out (vty, "Built on: %s %s\n", __DATE__, __TIME__);
	return CMD_SUCCESS;
}

DEFUN (show_cpuinfo,
	   show_cpuinfo_cmd,
	   "show cpuinfo",
	   SHOW_STR
	   "Displays the cpu information\n")
{
	char info[4096];
	FILE *fp;

	fp = fopen("/proc/cpuinfo", "r");
	if (fp) {
		memset(info, 0, sizeof(info));
		fread(info, sizeof(char), sizeof(info), fp);
		fclose(fp);
		vty_out (vty, "%s\n", info);
	}
	return CMD_SUCCESS;
}

DEFUN (show_meminfo,
	   show_meminfo_cmd,
	   "show meminfo",
	   SHOW_STR
	   "Displays the memory information\n")
{
	char info[4096];
	FILE *fp;

	fp = fopen("/proc/meminfo", "r");
	if (fp) {
		memset(info, 0, sizeof(info));
		fread(info, sizeof(char), sizeof(info), fp);
		fclose(fp);
		vty_out (vty, "%s\n", info);
	}
	return CMD_SUCCESS;
}

DEFUN (show_interruptsinfo,
	   show_interruptsinfo_cmd,
	   "show interruptsinfo",
	   SHOW_STR
	   "Displays the interrupts information\n")
{
	char info[4096];
	FILE *fp;

	fp = fopen("/proc/interrupts", "r");
	if (fp) {
		memset(info, 0, sizeof(info));
		fread(info, sizeof(char), sizeof(info), fp);
		fclose(fp);
		vty_out (vty, "%s\n", info);
	}
	return CMD_SUCCESS;
}

DEFUN (show_uptime,
        show_uptime_cmd,
        "show uptime",
        SHOW_STR
        "Displays the system uptime\n")
{
	system("uptime");
	return CMD_SUCCESS;
}

/* Help display function for all node. */
DEFUN (config_list,
       config_list_cmd,
       "list",
       "Print command list\n")
{
	int i;
	struct cmd_node *cnode = vector_slot (cmdvec, vty->node);
	struct cmd_element *cmd;

	for (i = 0; i < vector_max (cnode->cmd_vector); i++)
		if ((cmd = vector_slot (cnode->cmd_vector, i)) != NULL)
			vty_out (vty, "  %s%s", cmd->string, VTY_NEWLINE);
	return CMD_SUCCESS;
}

/* Write current configuration into file. */
DEFUN (config_write_file, 
       config_write_file_cmd,
       "write file",  
       "Write running configuration to memory, network, or terminal\n"
       "Write to configuration file\n")
{
	FILE *fp;

	if (host.config == NULL) {
		vty_out (vty, "%% Can't save to configuration file, using vtysh.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	fp = fopen(host.config, "w");
	if (fp == NULL) {
		vty_out (vty, "%% Can't open configuration file %s.%s", host.config, VTY_NEWLINE);
		return CMD_WARNING;
	}

	config_dump(fp);
	fflush(fp);
	vty_out (vty, "Configuration saved SUCCESS %s", VTY_NEWLINE);

	if (host.chpasswd) {
		char szCmd[1024];
		char password[256];

		strcpy(password, host.password);
		sprintf(szCmd, "echo -e \"%s\\n%s\" | passwd > /dev/null 2>&1",  password, password);
		system(szCmd);
		host.chpasswd = 0;
	}
	return CMD_SUCCESS;
}

ALIAS (config_write_file, 
       config_write_cmd,
       "write",  
       "Write running configuration to memory, network, or terminal\n"
       "Write configureation to the file(same as write file)\n");

ALIAS (config_write_file, 
       config_write_memory_cmd,
       "write memory",  
       "Write running configuration to memory, network, or terminal\n"
       "Write configuration to the file (same as write file)\n");

/* Write current configuration into the terminal. */
DEFUN (config_write_terminal,
       config_write_terminal_cmd,
       "write terminal",
       "Write running configuration to memory, network, or terminal\n"
       "Write to terminal\n")
{
	config_dump(stderr);
	return CMD_SUCCESS;
}

/* Write current configuration into the terminal. */
ALIAS (config_write_terminal,
       show_running_config_cmd,
       "show running-config",
       SHOW_STR
       "running configuration\n");

/* Write startup configuration into the terminal. */
DEFUN (show_startup_config,
       show_startup_config_cmd,
       "show startup-config",
       SHOW_STR
       "Contentes of startup configuration\n")
{
	char buf[BUFSIZ];
	FILE *confp;

	confp = fopen (host.config, "r");
	if (confp == NULL) {
		vty_out (vty, "%% Can't open configuration file [%s]%s",
				host.config, VTY_NEWLINE);
		return CMD_WARNING;
	}

	while (fgets (buf, BUFSIZ, confp)) {
		char *cp = buf;

		while (*cp != '\r' && *cp != '\n' && *cp != '\0')
			cp++;
		*cp = '\0';

		vty_out (vty, "%s%s", buf, VTY_NEWLINE);
	}

	fclose (confp);

	return CMD_SUCCESS;
}

/* Hostname configuration */
DEFUN (config_hostname, 
       hostname_cmd,
       "hostname WORD",
       "Set system's network name\n"
       "This system's network name\n")
{
	char szCmd[1024];

	if (!isalpha((int) *argv[0])) {
		vty_out (vty, "%% Please specify string starting with alphabet%s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	config_del_line_byleft(config_top, "hostname");
	config_add_line(config_top, "hostname %s", argv[0]);
	ENSURE_CONFIG(vty);

	if (host.name)
		XFREE (0, host.name);
	host.name = strdup (argv[0]);

	sethostname(host.name, strlen(host.name));

	sprintf(szCmd, "uci set system.@system[0].hostname='%s'", argv[0]);
	system(szCmd);
	system("uci commit system");
	system("/etc/init.d/system reload");

	return CMD_SUCCESS;
}

DEFUN (config_no_hostname, 
       no_hostname_cmd,
       "no hostname [HOSTNAME]",
       NO_STR
       "Reset system's network name\n"
       "Host name of this router\n")
{
	if (host.name)
		XFREE (0, host.name);
	host.name = NULL;
	config_del_line_byleft(config_top, "hostname ");

#if defined(NANO_R2S_PLUS)
	system("uci set system.@system[0].hostname='nano-r2s-plus'");
	system("uci commit system");
	system("/etc/init.d/system reload");
#endif

	return CMD_SUCCESS;
}

#if 0
/* VTY enable password set. */
DEFUN (config_enable_password, enable_password_cmd,
       "enable password (8|) WORD",
       "Modify enable password parameters\n"
       "Assign the privileged level password\n"
       "Specifies a HIDDEN password will follow\n"
       "The HIDDEN 'enable' password string\n")
{
	/* Argument check. */
	if (argc == 0) {
		vty_out (vty, "%% Please specify password.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	/* Crypt type is specified. */
	if (argc == 2) {
		if (*argv[0] == '8') {
			if (host.enable)
				XFREE (0, host.enable);
			host.enable = NULL;

			if (host.enable_encrypt)
				XFREE (0, host.enable_encrypt);
			host.enable_encrypt = XSTRDUP (0, argv[1]);

			config_del_line_byleft(config_top, "enable password");
			config_add_line(config_top, "enable password 8 %s", host.enable_encrypt);
			return CMD_SUCCESS;
		} else {
			vty_out (vty, "%% Unknown encryption type.%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
	}

	if (!isalnum ((int) *argv[0])) {
		vty_out (vty, "%% Please specify string starting with alphanumeric%s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	if (strlen(argv[0]) < 2) {
		vty_out (vty, "%% Please specify a long password%s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	if (host.enable)
		XFREE (0, host.enable);
	host.enable = NULL;

	if (host.enable_encrypt)
		XFREE (0, host.enable_encrypt);
	host.enable_encrypt = NULL;

	/* Plain password input. */
	config_del_line_byleft(config_top, "enable password");
	host.enable_encrypt = XSTRDUP (0, zencrypt (argv[0]));
	config_add_line(config_top, "enable password 8 %s", host.enable_encrypt);
	return CMD_SUCCESS;
}

ALIAS (config_enable_password,
       enable_password_text_cmd,
       "enable password LINE",
       "Modify enable password parameters\n"
       "Assign the privileged level password\n"
       "The UNENCRYPTED (cleartext) 'enable' password\n");

/* VTY enable password delete. */
DEFUN (no_config_enable_password, no_enable_password_cmd,
       "no enable password",
       NO_STR
       "Modify enable password parameters\n"
       "Assign the privileged level password\n")
{
	config_del_line_byleft(config_top, "enable password");

	if (host.enable)
		XFREE (0, host.enable);
	host.enable = NULL;

	if (host.enable_encrypt)
		XFREE (0, host.enable_encrypt);
	host.enable_encrypt = NULL;

	return CMD_SUCCESS;
}

DEFUN (config_password, password_cmd,
       "password (8|) WORD",
       "Modify password parameters\n"
       "Specifies a HIDDEN password will follow\n"
       "The HIDDEN password string\n")
{
	/* Argument check. */
	if (argc == 0) {
		vty_out (vty, "%% Please specify password.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	/* Crypt type is specified. */
	if (argc == 2) {
		if (*argv[0] == '8') {
			if (host.password)
				XFREE (0, host.password);
			host.password = NULL;

			if (host.password_encrypt)
				XFREE (0, host.password_encrypt);
			host.password_encrypt = XSTRDUP (0, argv[1]);

			config_del_line_byleft(config_top, "password");
			config_add_line(config_top, "password 8 %s", host.password_encrypt);
			return CMD_SUCCESS;
		} else {
			vty_out (vty, "%% Unknown encryption type.%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
	}

	if (!isalnum ((int) *argv[0])) {
		vty_out (vty, "%% Please specify string starting with alphanumeric%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if (strchr(argv[0], '"') ||
		strchr(argv[0], '`') ||
		strchr(argv[0], '<') ||
		strchr(argv[0], '>') ||
		strchr(argv[0], '|')) {
		vty_out(vty, "%% Password can't include special char\n");
		return CMD_WARNING;
	}
	if (strlen(argv[0]) < 2) {
		vty_out (vty, "%% Please specify a long password%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if (host.password)
		XFREE (0, host.password);
	host.password = NULL;

	if (host.password_encrypt)
		XFREE (0, host.password_encrypt);
	host.password_encrypt = NULL;

	/* Plain password input. */
	config_del_line_byleft(config_top, "password ");
	host.password = XSTRDUP(0, argv[0]);
	host.password_encrypt = XSTRDUP (0, zencrypt (argv[0]));
	config_add_line(config_top, "password 8 %s", host.password_encrypt);
	
	ENSURE_CONFIG(vty);
	host.chpasswd = 1;
	return CMD_SUCCESS;
}

ALIAS (config_password,
       password_text_cmd,
       "password LINE",
       "Modify password parameters\n"
       "The UNENCRYPTED (cleartext) password\n");
#endif

int cmd_common_init()
{
	/* Each node's basic commands. */
	cmd_install_element (VIEW_NODE, &show_version_cmd);
	cmd_install_element (VIEW_NODE, &config_exit_cmd);
	cmd_install_element (VIEW_NODE, &config_enable_cmd);

	cmd_install_element (VIEW_NODE, &show_cpuinfo_cmd);
	cmd_install_element (VIEW_NODE, &show_meminfo_cmd);
	cmd_install_element (VIEW_NODE, &show_interruptsinfo_cmd);
	cmd_install_element (VIEW_NODE, &show_uptime_cmd);

	cmd_install_element (ENABLE_NODE, &show_version_cmd);
	cmd_install_element (ENABLE_NODE, &config_exit_cmd);
	cmd_install_element (ENABLE_NODE, &config_list_cmd);
	cmd_install_element (ENABLE_NODE, &config_write_terminal_cmd);
	cmd_install_element (ENABLE_NODE, &config_write_file_cmd);
	cmd_install_element (ENABLE_NODE, &config_write_memory_cmd);
	cmd_install_element (ENABLE_NODE, &config_write_cmd);

	cmd_install_element (ENABLE_NODE, &show_cpuinfo_cmd);
	cmd_install_element (ENABLE_NODE, &show_meminfo_cmd);
	cmd_install_element (ENABLE_NODE, &show_interruptsinfo_cmd);
	cmd_install_element (ENABLE_NODE, &show_uptime_cmd);

	cmd_install_element (ENABLE_NODE, &config_disable_cmd);
	cmd_install_element (ENABLE_NODE, &config_terminal_cmd);
	cmd_install_element (ENABLE_NODE, &show_running_config_cmd);
	cmd_install_element (ENABLE_NODE, &show_startup_config_cmd);

	cmd_install_element (CONFIG_NODE, &show_version_cmd);
	cmd_install_element (CONFIG_NODE, &config_exit_cmd);
#if 0
	cmd_install_element (CONFIG_NODE, &config_end_cmd);
	cmd_install_element (CONFIG_NODE, &config_list_cmd);
#endif
	cmd_install_element (CONFIG_NODE, &config_write_terminal_cmd);
	cmd_install_element (CONFIG_NODE, &config_write_file_cmd);
	cmd_install_element (CONFIG_NODE, &config_write_memory_cmd);
	cmd_install_element (CONFIG_NODE, &config_write_cmd);
	cmd_install_element (CONFIG_NODE, &show_running_config_cmd);
	cmd_install_element (CONFIG_NODE, &show_startup_config_cmd);
	cmd_install_element (CONFIG_NODE, &hostname_cmd);
	cmd_install_element (CONFIG_NODE, &no_hostname_cmd);
#if 0
	cmd_install_element (CONFIG_NODE, &password_cmd);
	cmd_install_element (CONFIG_NODE, &password_text_cmd);
	cmd_install_element (CONFIG_NODE, &enable_password_cmd);
	cmd_install_element (CONFIG_NODE, &enable_password_text_cmd);
	cmd_install_element (CONFIG_NODE, &no_enable_password_cmd);
#endif

	return 0;
}

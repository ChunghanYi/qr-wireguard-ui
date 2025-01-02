/* !!!DONT EDIT THIS FILE!!!*/
int cmd_bridge_init();
int cmd_common_init();
int cmd_tool_init();
int cmd_ip_init();
int cmd_firewall_init();
int cmd_vpn_init();

void cmd_parse_init()
{
	cmd_bridge_init();
	cmd_common_init();
	cmd_tool_init();
	cmd_ip_init();
	cmd_firewall_init();
	cmd_vpn_init();
}

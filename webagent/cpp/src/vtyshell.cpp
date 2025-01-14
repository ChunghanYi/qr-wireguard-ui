/*
 * vtysh action routines
 * Copyright (c) 2024-2025 Chunghan Yi <chunghan.yi@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include "inc/server.h"
#include "inc/vtyshell.h"
#include "inc/pipe_ret_t.h"
#include "spdlog/spdlog.h"

namespace vtyshell {
	std::map<std::string, int> vtysh_cmd;

	void initializeVtyshMap() {
		vtysh_cmd["SET_HOST_NAME"]               = SET_HOST_NAME;
		vtysh_cmd["CHANGE_ADMIN_PASSWORD"]       = CHANGE_ADMIN_PASSWORD;
		vtysh_cmd["REBOOT_SYSTEM"]               = REBOOT_SYSTEM;

		vtysh_cmd["SET_ETHERNET_INTERFACE"]      = SET_ETHERNET_INTERFACE;
		vtysh_cmd["NO_SET_ETHERNET_INTERFACE"]   = NO_SET_ETHERNET_INTERFACE;
		vtysh_cmd["ADD_ROUTE_ENTRY"]             = ADD_ROUTE_ENTRY;
		vtysh_cmd["REMOVE_ROUTE_ENTRY"]          = REMOVE_ROUTE_ENTRY;

		vtysh_cmd["SET_WIREGUARD_INTERFACE"]     = SET_WIREGUARD_INTERFACE;
		vtysh_cmd["NO_SET_WIREGUARD_INTERFACE"]  = NO_SET_WIREGUARD_INTERFACE;
		vtysh_cmd["SET_WIREGUARD_GLOBAL_CONFIG"] = SET_WIREGUARD_GLOBAL_CONFIG;
		vtysh_cmd["ADD_WIREGUARD_PEER"]          = ADD_WIREGUARD_PEER;
		vtysh_cmd["REMOVE_WIREGUARD_PEER"]       = REMOVE_WIREGUARD_PEER;
	}

	std::vector<std::string> split(std::string s, std::string delimiter) {
		size_t pos_start = 0, pos_end, delim_len = delimiter.length();
		std::string token;
		std::vector<std::string> res;

		while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
			token = s.substr (pos_start, pos_end - pos_start);
			pos_start = pos_end + delim_len;
			res.push_back (token);
		}

		res.push_back (s.substr (pos_start));
		return res;
	}

	//Too C style :(
	bool getIPNetmask(std::string netinfo, char *ipstr, char *netmaskstr) {
		char *mask;
		int maskbits;
		struct in_addr addr;
		struct in_addr netmask;

		if (!(mask = strchr((char *)netinfo.c_str(),'/'))) {
			return false;
		}

		*mask++ = '\0';
		maskbits = atoi(mask);
		if (!inet_aton(netinfo.c_str(), &addr) || (maskbits > 30)) {
			return false;
		}

		netmask.s_addr = ntohl(0xffffffff << (32 - maskbits));

		sprintf(ipstr, "%s", inet_ntoa(addr));
		sprintf(netmaskstr, "%s", inet_ntoa(netmask));

		return true;
	}

	bool runCommand(const char *buf) {
		char command[256];
		sprintf(command, "/usr/bin/qrwg/vtysh -e \"%s\"", buf);
		system(command);
		return true;
	}

	bool doAction(std::string& s) {
		std::string KeyValue[16];
		std::string delimiter1 = "\n";
		std::string delimiter2 = ":=";

		std::vector<std::string> l = split(s, delimiter1);

		//l[1] => subcmd:=ADD_WIREGUARD_PEER\n
		std::vector<std::string> subcmd = split(l[1], delimiter2);

		//l[2] => field_count:=X\n
		std::vector<std::string> fcount = split(l[2], delimiter2);
		int count = std::stoi(fcount[1]);
		for (int i=0; i<count; i++) {
			//l[3+i] => keyN:=XXXXXXXXXXXX\n
			std::vector<std::string> keyval = split(l[i+3], delimiter2);
			KeyValue[i] = keyval[1];
		}

		bool ok_flag = true;
		char scmd[256];
		char ipstr[16], netmaskstr[16];

		switch (vtysh_cmd[subcmd[1]]) {
			case SET_HOST_NAME:
				//CLI: hostname WORD
				spdlog::debug(">>> SET_HOST_NAME !!!");
				sprintf(scmd, "hostname %s", KeyValue[0].c_str());
				break;

			case REBOOT_SYSTEM:
				//CLI: reboot
				spdlog::debug(">>> REBOOT_SYSTEM !!!");
				sprintf(scmd, "reboot");
				break;

			case SET_ETHERNET_INTERFACE:
				//CLI: ip address ETHNAME A.B.C.D A.B.C.D
				spdlog::debug(">>> SET_ETHERNET_INTERFACE !!!");
				if (getIPNetmask(KeyValue[0], ipstr, netmaskstr)) {
					sprintf(scmd, "ip address %s %s %s", KeyValue[1].c_str(), ipstr, netmaskstr);
				} else {
					return false;
				}
				break;

			case NO_SET_ETHERNET_INTERFACE:
				//CLI: no ip address ETHNAME
				spdlog::debug(">>> NO_SET_ETHERNET_INTERFACE !!!");
				sprintf(scmd, "no ip address %s", KeyValue[0].c_str());
				break;

			case ADD_ROUTE_ENTRY:
				//CLI: ip route A.B.C.D A.B.C.D A.B.C.D ETHNAME
				spdlog::debug(">>> ADD_ROUTE_ENTRY !!!");
				sprintf(scmd, "ip route %s %s %s %s",
						KeyValue[1].c_str(),
						KeyValue[2].c_str(),
						KeyValue[3].c_str(),
						KeyValue[0].c_str());
				break;

			case REMOVE_ROUTE_ENTRY:
				//CLI: no ip route A.B.C.D A.B.C.D
				spdlog::debug(">>> REMOVE_ROUTE_ENTRY !!!");
				sprintf(scmd, "no ip route %s %s",
						KeyValue[1].c_str(),
						KeyValue[2].c_str());
				break;

			case SET_WIREGUARD_INTERFACE:
				//CLI: ip address ETHNAME A.B.C.D A.B.C.D
				spdlog::debug(">>> SET_WIREGUARD_INTERFACE !!!");
				if (getIPNetmask(KeyValue[0], ipstr, netmaskstr)) {
					sprintf(scmd, "ip address wg0 %s %s", ipstr, netmaskstr);
				} else {
					return false;
				}
				break;

			case NO_SET_WIREGUARD_INTERFACE:
				//CLI: no ip address ETHNAME
				spdlog::debug(">>> NO_SET_WIREGUARD_INTERFACE !!!");
				sprintf(scmd, "no ip address wg0");
				break;

			case SET_WIREGUARD_GLOBAL_CONFIG:
				//CLI: <NOT IMPLEMENTED>
				spdlog::debug(">>> SET_WIREGUARD_GLOBAL_CONFIG !!!");
				return true;

			case ADD_WIREGUARD_PEER:
				//CLI: wg peer PUBLICKEY allowed-ips WORD endpoint A.B.C.D:PORT persistent-keepalive NUM
				spdlog::debug(">>> ADD_WIREGUARD_PEER !!!");
				sprintf(scmd, "wg peer %s allowed-ips %s endpoint %s persistent-keepalive 25",
						KeyValue[0].c_str(),
						KeyValue[1].c_str(),
						KeyValue[2].c_str());
				break;

			case REMOVE_WIREGUARD_PEER:
				//CLI: no wg peer PUBLICKEY
				spdlog::debug(">>> REMOVE_WIREGUARD_PEER !!!");
				sprintf(scmd, "no wg peer %s", KeyValue[0].c_str());
				break;

			default:
				spdlog::info(">>> UNKNOWN SUBCMD !!!");
				return false;
		}

		ok_flag = runCommand(scmd);
		if (ok_flag) {
			char command[256];
			sprintf(command, "/usr/bin/qrwg/vtysh -e \"write\"");
			system(command);
		}
		return true;
	}
}

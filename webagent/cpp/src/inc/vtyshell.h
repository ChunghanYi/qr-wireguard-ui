/*
 * Copyright (c) 2024-2025 Chunghan Yi <chunghan.yi@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>
#include <vector>

namespace vtyshell {
	enum vtyshcmd {
		SET_HOST_NAME               = 100,
		CHANGE_ADMIN_PASSWORD       = 101,
		REBOOT_SYSTEM               = 102,

		SET_ETHERNET_INTERFACE      = 110,
		NO_SET_ETHERNET_INTERFACE   = 111,
		ADD_ROUTE_ENTRY             = 112,
		REMOVE_ROUTE_ENTRY          = 113,

		SET_WIREGUARD_INTERFACE     = 120,
		NO_SET_WIREGUARD_INTERFACE  = 121,
		SET_WIREGUARD_GLOBAL_CONFIG = 122,
		ADD_WIREGUARD_PEER          = 123,
		REMOVE_WIREGUARD_PEER       = 124,
	};

	void initializeVtyshMap();
	std::vector<std::string> split(std::string s, std::string delimiter);
	bool runCommand(std::string& fullcmd);
	bool doAction(std::string& s);
};

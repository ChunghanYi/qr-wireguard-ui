/*
 * Copyright (c) 2019 Elhay Rauper
 * Copyright (c) 2024-2025 Chunghan Yi <chunghan.yi@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>
#include <functional>
#include "client.h"

struct server_observer_t {
	std::string wantedIP = "";
	std::function<bool(const std::string &clientIP, const char *msg, size_t size)> incomingPacketHandler;
	std::function<bool(const Client &client, const char *msg, size_t size)> incomingSinglePacketHandler;
	std::function<void(const std::string &ip, const std::string &msg)> disconnectionHandler;
};

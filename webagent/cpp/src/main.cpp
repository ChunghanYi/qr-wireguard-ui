/*
 * Startup Codes for Web Backend Daemon
 * Copyright (c) 2024-2025 Chunghan Yi <chunghan.yi@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <csignal>
#include <vector>
#include "inc/server.h"
#include "inc/common.h"
#include "inc/vtyshell.h"
#include "spdlog/spdlog.h"

// tcp server instance
TcpServer server;
// declare a server observer which will receive incomingPacketHandler messages.
server_observer_t observer;

const std::string versionString { "v0.9.0" }; 

static void printUsage() {
	std::cout << "Usage: web-agentd [OPTION]" << "\n";
	std::cout << "Options" << "\n";
	std::cout << " -f, --foreground    in foreground" << "\n";
	std::cout << " -d, --daemon        fork in background" << "\n";
	std::cout << " -v, --version       show version information and exit" << "\n\n";
	exit(EXIT_FAILURE);
}

static void printVersion() {
	std::cout << "wbe-agentd " << versionString << "\n";
	std::cout << "Copyright (c) 2024-2025 Chunghan Yi <chunghan.yi@gmail.com>" << "\n";
	exit(EXIT_SUCCESS);
}

static void sig_handler(int sig) {
	switch (sig) {
		case SIGINT:
		case SIGTERM:
		case SIGQUIT:
			spdlog::info(">>> Received signal {}, exiting...", sig);
			server.setTerminate(true);
			server.close();
			exit(EXIT_SUCCESS);
			break;
		default:
			break;
	}
}

static void daemonize(void) {
	int r;

	r = daemon(0, 0);
	if (r != 0) {
		spdlog::info("Unable to daemonize");
		exit(EXIT_FAILURE);
	}
}

constexpr unsigned int hashMagic(const char *str) {
	return str[0] ? static_cast<unsigned int>(str[0]) + 0xEDB8832Full * hashMagic(str + 1) : 8603;
}

bool onIncomingMsg_basedIP(const std::string &clientIP, const char *msg, size_t size) {
	return true;
}

bool onIncomingMsg_basedSocket(const Client &client, const char *msg, size_t size) {
	char buffer[MAX_PACKET_SIZE] {};
	memcpy(buffer, msg, size);

	std::string s{buffer};

	std::string delimiter1 = "\n";
	std::string delimiter2 = ":=";

	std::vector<std::string> l = vtyshell::split(s, delimiter1);
	std::vector<std::string> x = vtyshell::split(l[0], delimiter2);	//l[0] --> cmd:=HELLO\n

	if (x[1] == "HELLO") {
		spdlog::info(">>> cmd:=HELLO message received.");
		if (vtyshell::doAction(s)) {
			return server.send_OK(client);
		} else {
			return server.send_NOK(client);
		}
	} else if (x[1] == "BYE") {
		spdlog::info(">>> cmd:=BYE message received.");
		return server.send_OK(client);
	} else {
		spdlog::info(">>> UNKNOWN message received.");
		return server.send_NOK(client);
	}
}

void onClientDisconnected(const std::string &ip, const std::string &msg) {
	spdlog::info("Client: {} disconnected. Reason: {}", ip, msg);
}

void acceptClients() {
	try {
		std::string clientIP = server.acceptClient(0);
	} catch (const std::runtime_error &error) {
		spdlog::error("Accepting client failed: {}", error.what());
	}
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printUsage();
	}

	switch (hashMagic(argv[1])) {
		case hashMagic("-v"):
		case hashMagic("--version"):
			printVersion();
			break;

		case hashMagic("-f"):
		case hashMagic("--foreground"):
			break;

		case hashMagic("-d"):
		case hashMagic("--daemon"):
			daemonize();
			break;

		default:
			printUsage();
			break;
	}

	signal(SIGINT, sig_handler);
	signal(SIGQUIT, sig_handler);
	signal(SIGTERM, sig_handler);

	spdlog::info("Starting the web-agentd(tcp port 51821)...");
	vtyshell::initializeVtyshMap();
	pipe_ret_t startRet = server.start(51821);
	if (!startRet.isSuccessful()) {
		spdlog::error("Server setup failed: {}", startRet.message());
		return EXIT_FAILURE;
	}

	// configure and register observer
	observer.incomingPacketHandler = onIncomingMsg_basedIP;
	observer.incomingSinglePacketHandler = onIncomingMsg_basedSocket;
	observer.disconnectionHandler = onClientDisconnected;
	observer.wantedIP = "127.0.0.1";
	server.subscribe(observer);

	while (!server.shouldTerminate()) {
		acceptClients();
	}

	server.close();
	spdlog::info("The web-agentd is stopped.");

	return EXIT_SUCCESS;
}

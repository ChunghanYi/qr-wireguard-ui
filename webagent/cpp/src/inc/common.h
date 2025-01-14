/*
 * Copyright (c) 2019 Elhay Rauper
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdio>

#define MAX_PACKET_SIZE 4096

namespace fd_wait {
	enum Result {
		FAILURE,
		TIMEOUT,
		SUCCESS
	};

	Result waitFor(const FileDescriptor &fileDescriptor, uint32_t timeoutSeconds = 1);
};

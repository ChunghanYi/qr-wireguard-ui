/*
 * Copyright (c) 2019 Elhay Rauper
 *
 * SPDX-License-Identifier: MIT
 */

#include <string>
#include <cstdint>
#include "inc/file_descriptor.h"
#include "inc/common.h"
#include "inc/pipe_ret_t.h"

#include <sys/select.h>

#define SELECT_FAILED -1
#define SELECT_TIMEOUT 0

namespace fd_wait {
	/**
	 * monitor file descriptor and wait for I/O operation
	 */
	Result waitFor(const FileDescriptor &fileDescriptor, uint32_t timeoutSeconds) {
		struct timeval tv;
		tv.tv_sec = timeoutSeconds;
		tv.tv_usec = 0;
		fd_set fds;

		FD_ZERO(&fds);
		FD_SET(fileDescriptor.get(), &fds);
		const int selectRet = select(fileDescriptor.get() + 1, &fds, nullptr, nullptr, &tv);

		if (selectRet == SELECT_FAILED) {
			return Result::FAILURE;
		} else if (selectRet == SELECT_TIMEOUT) {
			return Result::TIMEOUT;
		}
		return Result::SUCCESS;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////

pipe_ret_t pipe_ret_t::failure(const std::string &msg) {
	return pipe_ret_t(false, msg);
}

pipe_ret_t pipe_ret_t::success(const std::string &msg) {
	return pipe_ret_t(true, msg);
}

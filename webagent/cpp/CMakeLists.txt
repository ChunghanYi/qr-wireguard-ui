# SPDX-License-Identifier: MIT
#
# Copyright (c) 2024-2025 Chunghan Yi <chunghan.yi@gmail.com>

cmake_minimum_required(VERSION 3.8.1)
project(webagent)

find_package (Threads)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11")

add_library(spdlog STATIC IMPORTED)
set_target_properties(spdlog
	PROPERTIES IMPORTED_LOCATION
	${CMAKE_SOURCE_DIR}/external/lib/libspdlog.a)

include_directories(${CMAKE_SOURCE_DIR}/external/lib/include)

add_executable(web-agentd
	${CMAKE_SOURCE_DIR}/src/main.cpp
	${CMAKE_SOURCE_DIR}/src/server.cpp
	${CMAKE_SOURCE_DIR}/src/client.cpp
	${CMAKE_SOURCE_DIR}/src/vtyshell.cpp
	${CMAKE_SOURCE_DIR}/src/common.cpp)

target_link_libraries (web-agentd spdlog ${CMAKE_THREAD_LIBS_INIT})

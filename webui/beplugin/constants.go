/*
 * Copyright (c) 2024 Chunghan Yi <chunghan.yi@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package beplugin

// WebUI Backend Protocol #1 - Main Commands
const (
	HELLO     = iota + 10   // 10
	OK                      // 11
	NOK                     // 12
	BYE                     // 13
)

// WebUI Backend Protocol #2 - Sub Commands
const (
    SET_HOST_NAME            = iota + 100  // 100
    CHANGE_ADMIN_PASSWORD                  // 101
    REBOOT_SYSTEM                          // 102

    SET_WIREGUARD_INTERFACE  = iota + 110  // 110
    ADD_WIREGUARD_PEER                     // 111
    REMOVE_WIREGUARD_PEER                  // 112

    SET_ETHERNET_INTERFACE   = iota + 120  // 120
    ADD_ROUTE_ENTRY                        // 121
    REMOVE_ROUTE_ENTRY                     // 122
    ADD_DEFAULT_GATEWAY                    // 123
    REMOVE_DEFAULT_GATEWAY                 // 124
)

const (
	SERVER_PORT_DEFAULT = ":51821"
)

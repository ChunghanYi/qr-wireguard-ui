/*
 * Message format between webserver and webagent
 * Copyright (c) 2024 Chunghan Yi <chunghan.yi@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package model

//WebUI -> backend daemon
type RequestMessage struct {
	Cmd          string     `cmd:=HELLO`
	SubCmd       string     `subcmd:=wg_peer_add`
	FieldCount   string     `field_count:=N`
	KeyValue     [16]string `keyN:=valueN`
}

//WebUI <- backend daemon
type ReplyMessage struct {
	Cmd          string    `cmd:=OK`
}

/*
 * wireguard-ui backend plugin routines
 * Copyright (c) 2024 Chunghan Yi <chunghan.yi@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package beplugin

import (
	"fmt"
	"net"
	"bytes"
	"encoding/gob"
)

func handleConnection(c net.Conn, smsg *RequestMessage) bool {
	defer c.Close()

	var rmsg ReplyMessage
	var network bytes.Buffer
	enc := gob.NewEncoder(&network)
	err := enc.Encode(*smsg)
	if err != nil {
		fmt.Println("encoding error");
		fmt.Println(err)
		return false
	}
	_, err = c.Write(network.Bytes())
	if err != nil {
		fmt.Println(err)
	}

	data := make([]byte, 4096)
	n, err := c.Read(data)
	if err != nil {
		fmt.Println(err)
		return false
	}

	r := bytes.NewReader(data[:n])
	dec := gob.NewDecoder(r)
	err = dec.Decode(&rmsg)
	if err != nil {
		fmt.Println("decode:", err)
		return false
	}

	if rmsg.Cmd == "cmd:=OK" {
		return true
	} else {
		return false
	}
}

func Xsend(smsg *RequestMessage) bool {
	conn, err := net.Dial("tcp", SERVER_PORT_DEFAULT)
	if err != nil {
		fmt.Println(err)
		return false
	}
	return handleConnection(conn, smsg)
}

/* usage example:
{
	var smsg beplugin.RequestMessage

	smsg.Cmd = "cmd:=HELLO\n"
	smsg.SubCmd = "subcmd:=ADD_WIREGUARD_PEER\n"
	smsg.FieldCount = "field_count:=5\n"
	smsg.KeyValue[0] = "key1:=uiu4q6TpkI9vW9zz0mCF72ZD6CbIlc9EczAOhMHhVgk=\n"
	smsg.KeyValue[1] = "key2:=xxx4q6TpkI9vW9zz0mCF72ZD6CbIlc9EczAOhMHhVgk=\n"
	smsg.KeyValue[2] = "key3:=192.168.1.0/24,10.0.0.0/8,0.0.0.0/0\n"
	smsg.KeyValue[3] = "key4:=1.1.1.1:12345\n"
	smsg.KeyValue[4] = "key5:=-\n"

	if beplugin.Xsend(&smsg) {
		fmt.Println("Message operation is OK.")
	} else {
		fmt.Println("Message operation is failed.")
	}
}
*/

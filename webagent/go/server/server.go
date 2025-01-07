/*
 * Server routines
 * Copyright (c) 2024 Chunghan Yi <chunghan.yi@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package server

import (
	"fmt"
	"log"
	"net"
	"strings"
	"errors"
	"bytes"
	"encoding/gob"

	"web-agentd/model"
)

var EndServer bool

//Server interface
type Server interface {
	Run() error
	Close() error
}

//var udp Server
//type UDPServer struct {
//	addr   string
//	server *net.UDPConn
//}

var tcp Server

type TCPServer struct {
	addr   string
	server net.Listener
}

func NewServer(protocol, addr string) (Server, error) {
	switch strings.ToLower(protocol) {
	case "tcp":
		return &TCPServer{
			addr: addr,
		}, nil
//
//	case "udp":
//		return &UDPServer{
//			addr: addr,
//		}, nil
//
	}
	return nil, errors.New("Invalid protocol")
}

////////////////////////////////////////////////////////////////////////////
func (t *TCPServer) send_message(c net.Conn, msg_cmd string, xmsg *model.RequestMessage) {
	var smsg model.ReplyMessage
	var network bytes.Buffer

	smsg.Cmd = fmt.Sprintf("cmd:=%s", msg_cmd)

	enc := gob.NewEncoder(&network)
	err := enc.Encode(smsg)
	if err != nil {
		log.Fatalln(err)
		return
	}
	_, err = c.Write(network.Bytes())
	if err != nil {
		log.Fatalln(err)
		return
	}

	switch (msg_cmd) {
	case "OK":
		log.Println("<<< OK message sent.")
	case "NOK":
		log.Println("<<< NOK message sent.")
	default:
		log.Println("<<< UNKNOWN message sent.")
	}
}

func (t *TCPServer) send_OK(c net.Conn, xmsg *model.RequestMessage) {
    t.send_message(c, "OK", xmsg)
}

func (t *TCPServer) send_NOK(c net.Conn, xmsg *model.RequestMessage) {
	t.send_message(c, "NOK", xmsg)
}

func (t *TCPServer) handleConnection(c net.Conn) {
	defer c.Close()
	var rmsg model.RequestMessage

	data := make([]byte, 4096)

	for {
		n, err := c.Read(data)
		if err != nil {
			return
		}

		r := bytes.NewReader(data[:n])
		dec := gob.NewDecoder(r)
		err = dec.Decode(&rmsg)
		if err != nil {
			log.Fatalln("decoding error:", err)
			return
		}

		temp := strings.Split(rmsg.Cmd, ":=")  //cmd:=HELLO\n
		x := strings.TrimSuffix(temp[1], "\n")
		switch (x) {
		case "HELLO":
			log.Printf(">>> cmd:=HELLO message received.\n")
			flag := doAction(&rmsg)
			if flag == true {
				t.send_OK(c, &rmsg)
			} else {
				t.send_NOK(c, &rmsg)
			}
			return
		case "BYE":
			log.Println(">>> cmd:=BYE message received.")
			t.send_OK(c, &rmsg)
			return
		default:
			log.Println(">>> UNKNOWN message received.")
			t.send_NOK(c, &rmsg)
			return
		}
	}
}

func (t *TCPServer) Run() (err error) {
	t.server, err = net.Listen("tcp", t.addr)
	if err != nil {
		return err
	}
	defer t.Close()

	for {
		conn, err := t.server.Accept()
		if err != nil {
			err = errors.New("could not accept connection")
			break
		}
		if conn == nil {
			err = errors.New("could not create connection")
			break
		}
		go t.handleConnection(conn)
	}
	return
}

func (t *TCPServer) Close() (err error) {
	return t.server.Close()
}

func Start_Server() (Server, error) {
	var err error
	tcp, err = NewServer("tcp", SERVER_PORT_DEFAULT)
	if err != nil {
		log.Fatalln("error starting TCP server")
		return tcp, err
	}

	go func() {
		tcp.Run()
	}()

	return tcp, err
}

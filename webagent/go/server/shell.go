/*
 * vtysh action routines
 * Copyright (c) 2024 Chunghan Yi <chunghan.yi@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package server

import (
	"fmt"
	"strconv"
	"strings"
	"net"
	"log"
	"os"
	"os/exec"

	"web-agentd/model"
)

// Run a shell script
func runCommand(fullcmd string) bool {
	file, err := os.Create(TEMP_SHELL_SCRIPT_PATH)
	if err != nil {
		log.Fatalln(err)
		return false
	}
	defer file.Close()

	s := fmt.Sprintf("#!/bin/sh\n/usr/bin/qrwg/vtysh -e \"%s\"\n", fullcmd)
	_, err = file.Write([]byte(s))
	if err != nil {
		log.Fatalln(err)
		return false
	}

	err = os.Chmod(TEMP_SHELL_SCRIPT_PATH, 0755)
	if err != nil {
		log.Fatalln(err)
		return false
	}

	app := TEMP_SHELL_SCRIPT_PATH
	c := exec.Command(app)
	c.Stdout = os.Stdout
	c.Stderr = os.Stderr
	c.Run()

	return true
}

func doAction(rmsg *model.RequestMessage) bool {
	var KeyValue [16]string
	var scmd string
	ok_flag := true

	temp := strings.Split(rmsg.SubCmd, ":=")  //subcmd:=ADD_WIREGUARD_PEER\n
	subcmd := strings.TrimSuffix(temp[1], "\n")
	log.Printf(">>> subcmd:=%s\n", subcmd)

	temp = strings.Split(rmsg.FieldCount, ":=")  //field_count:=X\n
	t := strings.TrimSuffix(temp[1], "\n")
	count, err := strconv.Atoi(t)
	if err != nil {
		log.Fatalln("strconv.Atoi error:", err)
		return false
	}
	log.Printf(">>> field_count:=%d\n", count)

	for i := 0; i < count; i++ {
		temp = strings.Split(rmsg.KeyValue[i], ":=")  //keyN:=XXXXXXXXXXXX\n
		t = strings.TrimSuffix(temp[1], "\n")
		log.Printf(">>> key%d:=%s\n", i, t)
		KeyValue[i] = t
	}

	switch subcmd {
	//CLI: hostname WORD
	case "SET_HOST_NAME":
		scmd = fmt.Sprintf("hostname %s", KeyValue[0])
		ok_flag = runCommand(scmd)

	//CLI: reboot
	case "REBOOT_SYSTEM":
		scmd = fmt.Sprintf("reboot")
		ok_flag = runCommand(scmd)

	//CLI: ip address ETHNAME A.B.C.D A.B.C.D
	case "SET_ETHERNET_INTERFACE":
		ip := strings.Split(KeyValue[0], "/")  // 172.16.1.254/24
		_, ipnet, err := net.ParseCIDR(KeyValue[1])
		if err != nil {
			log.Fatalln("failed parsing CIDR address: ", err)
			return false
		} else {
			scmd = fmt.Sprintf("ip address %s %s %s", KeyValue[0], ip[0], net.IP(ipnet.Mask))
			ok_flag = runCommand(scmd)
		}

	//CLI: no ip address ETHNAME
	case "NO_SET_ETHERNET_INTERFACE":
		scmd = fmt.Sprintf("no ip address %s", KeyValue[0])
		ok_flag = runCommand(scmd)

	//CLI: ip route A.B.C.D A.B.C.D A.B.C.D ETHNAME
	case "ADD_ROUTE_ENTRY":
		scmd = fmt.Sprintf("ip route %s %s %s %s",
				KeyValue[1], KeyValue[2], KeyValue[3], KeyValue[0])
		ok_flag = runCommand(scmd)

	//CLI: no ip route A.B.C.D A.B.C.D
	case "REMOVE_ROUTE_ENTRY":
		scmd = fmt.Sprintf("no ip route %s %s", KeyValue[1], KeyValue[2])
		ok_flag = runCommand(scmd)

	//CLI: ip address ETHNAME A.B.C.D A.B.C.D
	case "SET_WIREGUARD_INTERFACE":
		ip := strings.Split(KeyValue[0], "/")  // 172.16.1.254/24
		_, ipnet, err := net.ParseCIDR(KeyValue[0])
		if err != nil {
			log.Fatalln("failed parsing CIDR address: ", err)
			return false
		} else {
			scmd = fmt.Sprintf("ip address wg0 %s %s", ip[0], net.IP(ipnet.Mask))
			ok_flag = runCommand(scmd)
		}

	//CLI: no ip address ETHNAME
	case "NO_SET_WIREGUARD_INTERFACE":
		scmd = fmt.Sprintf("no ip address wg0")
		ok_flag = runCommand(scmd)

	//CLI:
	case "SET_WIREGUARD_GLOBAL_CONFIG":
		//ok_flag = runCommand(scmd)

	//CLI: wg peer PUBLICKEY allowed-ips WORD endpoint A.B.C.D:PORT persistent-keepalive NUM
	case "ADD_WIREGUARD_PEER":
		scmd = fmt.Sprintf("wg peer %s allowed-ips %s endpoint %s persistent-keepalive 25",
				KeyValue[0], KeyValue[1], KeyValue[2])
		ok_flag = runCommand(scmd)

	//CLI: no wg peer PUBLICKEY
	case "REMOVE_WIREGUARD_PEER":
		scmd = fmt.Sprintf("no wg peer %s", KeyValue[0])
		ok_flag = runCommand(scmd)

	default:
		log.Printf("*** Oops! UNKNOWN(%s) subcmd received.\n", subcmd)
		return false
	}
	log.Printf(">>> scmd = [%s]\n", scmd)

	//write it to vtysh configuration file
	if ok_flag == true {
		app := "/usr/bin/qrwg/vtysh"
		arg0 := "-e"
		arg1 := "write"
		cmd := exec.Command(app, arg0, arg1)
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		cmd.Run()
	}
	return ok_flag
}

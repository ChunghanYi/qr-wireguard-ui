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
	"log"
	"os"
	"os/exec"

	"web-agentd/model"
)

// Run a shell script
func runCommand(fullcmd string) bool {
	file, err := os.Create(TEMP_SHELL_SCRIPT_PATH)
	if err != nil {
		fmt.Println(err)
		return false
	}

	s := fmt.Sprintf("#!/bin/sh\n/sbin/vtysh -e \"%s\"\n", fullcmd)
	_, err = file.Write([]byte(s))
	if err != nil {
		fmt.Println(err)
		return false
	}
	file.Close()

	err = os.Chmod(TEMP_SHELL_SCRIPT_PATH, 0755)
	if err != nil {
		fmt.Println(err)
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
	var KeyValue [16]string  //TDB
	ok_flag := true

	temp := strings.Split(rmsg.SubCmd, ":=")  //subcmd:=ADD_WIREGUARD_PEER\n
	subcmd := strings.TrimSuffix(temp[1], "\n")
	fmt.Printf(">>> subcmd:=%s\n", subcmd)

	temp = strings.Split(rmsg.FieldCount, ":=")  //field_count:=X\n
	t := strings.TrimSuffix(temp[1], "\n")
	count, err := strconv.Atoi(t)
	if err != nil {
		log.Println("strconv.Atoi error:", err)
		return false
	}
	fmt.Printf(">>> field_count:=%d\n", count)

	for i := 0; i < count; i++ {
		temp = strings.Split(rmsg.KeyValue[i], ":=")  //keyN:=XXXXXXXXXXXX\n
		t = strings.TrimSuffix(temp[1], "\n")
		fmt.Printf(">>> key%d:=%s\n", i, t)
		KeyValue[i] = t
	}

	var scmd string
	switch subcmd {
	case "SET_HOST_NAME":
		scmd = fmt.Sprintf("hostname %s", KeyValue[0])
		fmt.Printf(">>> scmd ---> [%s]\n", scmd)
		//ok_flag = runCommand(scmd)

	case "CHANGE_ADMIN_PASSWORD":
		scmd = fmt.Sprintf("passwd %s %s", KeyValue[0], KeyValue[1])
		//ok_flag = runCommand(scmd)

	case "REBOOT_SYSTEM":
		scmd = fmt.Sprintf("reboot")
		//ok_flag = runCommand(scmd)

	case "SET_ETHERNET_INTERFACE":
		//ip address ETHNAME A.B.C.D A.B.C.D
		scmd = fmt.Sprintf("ip address %s %s %s",
				KeyValue[0], KeyValue[1], KeyValue[2])
		//ok_flag = runCommand(scmd)

	case "NO_SET_ETHERNET_INTERFACE":
		//no ip address ETHNAME
		scmd = fmt.Sprintf("no ip address %s", KeyValue[0])
		//ok_flag = runCommand(scmd)

	case "ADD_ROUTE_ENTRY":
		//ip route A.B.C.D A.B.C.D A.B.C.D ETHNAME
		scmd = fmt.Sprintf("ip route %s %s %s %s",
				KeyValue[0], KeyValue[1], KeyValue[2], KeyValue[3])
		//ok_flag = runCommand(scmd)

	case "REMOVE_ROUTE_ENTRY":
		//no ip route A.B.C.D A.B.C.D
		scmd = fmt.Sprintf("no ip route %s %s", KeyValue[0], KeyValue[1])
		//ok_flag = runCommand(scmd)

	case "SET_WIREGUARD_INTERFACE":
		//ip address ETHNAME A.B.C.D A.B.C.D
		scmd = fmt.Sprintf("ip address %s %s %s",
				KeyValue[0], KeyValue[1], KeyValue[2])
		//ok_flag = runCommand(scmd)

	case "NO_SET_WIREGUARD_INTERFACE":
		//no ip address ETHNAME
		scmd = fmt.Sprintf("no ip address %s", KeyValue[0])
		//ok_flag = runCommand(scmd)

	case "ADD_WIREGUARD_PEER":
		//wg peer PUBLICKEY allowed-ips (none|WORD) endpoint (none|FQDN:PORT|A.B.C.D:PORT) persistent-keepalive (off|NUM)
		scmd = fmt.Sprintf("wg peer %s allowed-ips %s endpoint %s persistent-keepalive %s",
				KeyValue[0], KeyValue[1], KeyValue[2], KeyValue[3])
		//ok_flag = runCommand(scmd)

	case "REMOVE_WIREGUARD_PEER":
		//no wg peer PUBLICKEY
		scmd = fmt.Sprintf("no wg peer %s", KeyValue[0])
		//ok_flag = runCommand(scmd)

	default:
		fmt.Printf("*** Oops! UNKNOWN subcmd received.\n")
		scmd = "UNKNOWN"
		ok_flag = false
	}
	fmt.Printf(">>> scmd = [%s]\n", scmd)

	//write it to vtysh configuration file
	if ok_flag == true {
		app := "vtysh"
		arg0 := "-e"
		arg1 := "write"
		cmd := exec.Command(app, arg0, arg1)
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		cmd.Run()
	}
	return ok_flag
}

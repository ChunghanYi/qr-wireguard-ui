/*
 * Startup Codes for Web Backend Daemon
 * Copyright (c) 2024 Chunghan Yi <chunghan.yi@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package main

import (
	"fmt"
	"log"
	"os"
	"os/signal"
	"strings"
	"time"
	"runtime"

	"golang.org/x/sys/unix"
	"web-agentd/server"
)

const (
	WBED_VERSION = "0.9.0"
	LOG_FILE = "/var/log/webagent.log"
	ENV_WG_PROCESS_FOREGROUND = "WG_PROCESS_FOREGROUND"
)
const (
	ExitSetupSuccess = 0
	ExitSetupFailed  = 1
)

func printUsage() {
	fmt.Printf("Usage : %s [-f|--foreground|-d|--daemon|-v|--version]\n", os.Args[0])
}

func printVersion() {
	fmt.Printf("wbe-agentd v%s for %s-%s.\n", WBED_VERSION, runtime.GOOS, runtime.GOARCH)
	fmt.Printf("Copyright (c) 2024-2025 Chunghan Yi <chunghan.yi@gmail.com>\n")
}

func main() {
	if len(os.Args) == 1 {
		printUsage()
		os.Exit(ExitSetupFailed)
	}

	var foreground bool
	switch strings.ToLower(os.Args[1]) {
	case "-v", "--version":
		printVersion()
		os.Exit(ExitSetupSuccess)

	case "-f", "--foreground":
		foreground = true

	case "-d", "--daemon":
		foreground = false

		// Specify the log file
		file, err := os.OpenFile(LOG_FILE, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
		if err != nil {
			log.Fatal(err)
		}
		defer file.Close()
		log.SetOutput(file)

	default:
		printUsage()
		os.Exit(ExitSetupFailed)
	}
	
	// TBD: daemonize the process
	if !foreground {
	}

	// Create a channel for OS signal
	term := make(chan os.Signal, 1)
	signal.Notify(term, unix.SIGTERM)
	signal.Notify(term, os.Interrupt)

	server.EndServer = false 
	go func() {
		select {
		case <-term:
			server.EndServer = true
		}
	}()

	log.Println("Starting the web-agentd...")
	s, _ := server.Start_Server()

	// wait for program to terminate
	for !server.EndServer {
		time.Sleep(1 * time.Second)
	}

	log.Println("Stopping the web-agentd...")
	s.Close() //socket close
}

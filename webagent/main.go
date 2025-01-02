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
	"runtime"

	"golang.org/x/sys/unix"
	"web-agentd/server"
)

const WBED_VERSION = "0.9.0"

func printUsage() {
	fmt.Printf("Usage : %s [-f|--foreground|-d|--daemon|-v|--version]\n", os.Args[0])
}

func printVersion() {
	fmt.Printf("wbe_agentd v%s for %s-%s.\n", WBED_VERSION, runtime.GOOS, runtime.GOARCH)
	fmt.Printf("Copyright (c) 2024 Chunghan Yi <chunghan.yi@gmail.com>\n")
}

func main() {
	if len(os.Args) == 1 {
		printUsage()
		os.Exit(1)
	}

	var foreground bool

	switch strings.ToLower(os.Args[1]) {
	case "version":
		printVersion()
		os.Exit(0)

	case "-f", "--foreground":
		foreground = true

	case "-d", "--daemon":
		foreground = false
		return

	default:
		printUsage()
		os.Exit(1)
	}

	if !foreground {
		// TODO: daemonize the process
	}

	term := make(chan os.Signal, 1)

	server.Start_Server()

	// wait for program to terminate
	signal.Notify(term, unix.SIGTERM)
	signal.Notify(term, os.Interrupt)

	select {
	case <-term:
	}

	log.Printf("Shutting down...\n")
}

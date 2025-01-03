#!/bin/sh

rm wireguard-ui
GOOS=linux GOARCH=arm64 go build -o wireguard-ui

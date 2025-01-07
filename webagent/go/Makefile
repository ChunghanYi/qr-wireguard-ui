PREFIX ?= /usr
DESTDIR ?=
BINDIR ?= $(PREFIX)/bin
#export GO111MODULE := on

LDFLAGS=-ldflags "-s -w"

all: web-agentd

MAKEFLAGS += --no-print-directory

web-agentd: $(wildcard *.go) $(wildcard */*.go)
	go build $(LDFLAGS) -v -o "$@"

install: web-agentd
	@install -v -d "$(DESTDIR)$(BINDIR)" && install -v -m 0755 "$<" "$(DESTDIR)$(BINDIR)/web-agentd"

test:
	go test ./...

clean:
	rm -f web-agentd

.PHONY: all clean test install web-agentd

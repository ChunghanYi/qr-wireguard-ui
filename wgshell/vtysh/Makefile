#
# Makefile for vtysh(Cisco style command line interface)
#

CFLAGS      = -Wall -g -I. -I./include -DNANO_R2S_PLUS
LIBS        = -L../../rootfs/lib/nanopi -lreadline -lcrypt -lncurses
TARGETDIR   = output

all:	prepare vtysh
prepare:
	@echo Parse the cmd directory ...
#	@/bin/dash parse.sh
OBJECT=${patsubst %.c, %.o, ${wildcard *.c cmd/*.c}}

vtysh: ${OBJECT}
	${CC} -o $@ $^ ${LIBS}

install: vtysh
	${STRIP} vtysh
#	cp vtysh ${TARGETDIR}/

.c.o:
.c.h:
clean:
	rm -f *.o cmd/*.o vtysh

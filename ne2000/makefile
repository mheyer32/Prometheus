CC ?= m68k-amigaos-gcc
LD ?= m68k-amigaos-gcc
STRIP = m68k-amigaos-strip

CFLAGS ?=
CFLAGS += -g -ggdb -noixemul -msmall-code -O2 -c -fomit-frame-pointer -I./sana_dev/include -I../PrometheusCard -I../PromLib/include

all: prm-rtl8029.device

prm-rtl8029.device: header.o driver.o
	$(CC) -ggdb -ramiga-dev -msmall-code -noixemul -nostartfiles -o $@_unstripped $^
	$(STRIP) $@_unstripped -o $@

header.o : header.s
	$(CC) $(CFLAGS) $^ -o $@

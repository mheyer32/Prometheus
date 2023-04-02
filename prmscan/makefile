#FIXME: We need to unify the place and naming of the public-facing Prometheus headers!

CC ?= m68k-amigaos-gcc

CFLAGS = -v -c -g -O2 -noixemul -msmall-code -fbaserel -fno-builtin -I./../PrometheusCard/ -I./../PromLib/include

all : PrmScan TestReadCfg

PrmScan: prmscan.o startup.o
	$(CC) -g -fbaserel -noixemul -o $@ $^

TestReadCfg: testreadcfg.o startup.o
	$(CC) -g -fbaserel -noixemul -o $@ $^

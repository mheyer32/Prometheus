#FIXME: We need to unify the place and naming of the public-facing Prometheus headers!

CC = m68k-amigaos-gcc

CFLAGS = -c -g -O2 -noixemul -msmall-code -fbaserel -fno-builtin -I./../PrometheusCard/ -I./../PromLib/include

all : DMATest FindRTL8139

DMATest: dmatest.o 
	$(CC) -g -fbaserel -noixemul -o $@ $^

FindRTL8139: rtl8139.o
	$(CC) -g -fbaserel -noixemul -o $@ $^

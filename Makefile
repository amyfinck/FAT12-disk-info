CC      = gcc
RM      = rm -f
RMD     = rm -f -r

default: all

all: main

main: main.c
	$(CC) -o diskinfo main.c

debug:
	$(CC) -g -o diskinfo main.c

clean:
	$(RM) diskinfo
	$(RMD) diskinfo.dSYM
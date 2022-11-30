CC      = gcc
RM      = rm -f
RMD     = rm -f -r

default: all

all: main

main: diskinfo.c
	$(CC) -o diskinfo diskinfo.c diskmethods.c
	$(CC) -o disklist disklist.c diskmethods.c
	$(CC) -o diskget diskget.c diskmethods.c
	$(CC) -o diskput diskput.c diskmethods.c

debug:
	$(CC) -g -o diskinfo diskinfo.c

clean:
	$(RM) diskinfo
	$(RMD) diskinfo.dSYM
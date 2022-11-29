#ifndef CSC360_ASSIGNMENT3_DISKMETHODS_H
#define CSC360_ASSIGNMENT3_DISKMETHODS_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include<sys/mman.h>
#include<string.h>

#define BYTES_PER_SECTOR 512
#define BYTES_PER_DIR_ENTRY 32

int getFatEntry(int position, char* p);
int getFreeSectorCount(char* p);
int getFilesOnDisk(char* p, int offset);

#endif //CSC360_ASSIGNMENT3_DISKMETHODS_H

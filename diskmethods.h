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
#define ROOT_OFFSET 19

int getFatEntry(char* p, int position);
int getFreeSectorCount(char* p);
int getFilesOnDisk(char* p, int offset);
void printCreationDateTime(int creationDate, int creationTime);
void printMonth(int month);
int getFileDirEntry(char* p, int offset, char* fileName);
void copyFileToLocalDir(char* p, char* fileCopy_p, int fileSize, int dirOffset, int dirEntry);
int getFileSize(char* p, int offset, int dirEntryNum);
int logicalToPhysicalSector(int logicalSector);

#endif //CSC360_ASSIGNMENT3_DISKMETHODS_H

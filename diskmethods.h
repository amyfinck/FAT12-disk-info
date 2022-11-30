#ifndef CSC360_ASSIGNMENT3_DISKMETHODS_H
#define CSC360_ASSIGNMENT3_DISKMETHODS_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include<sys/mman.h>
#include<string.h>

#define BYTES_PER_SECTOR 512
#define BYTES_PER_DIR_ENTRY 32
#define ROOT_OFFSET 19

int getDiskSize(char* p);
int getFatEntry(char* p, int position);
int getFreeSectorCount(char* p);
int getFilesOnDisk(char* p, int offset);
void printCreationDateTime(int creationDate, int creationTime);
void printMonth(int month);
int getFileDirEntry(char* p, int offset, char* fileName);
void copyFileToLocalDir(char* p, char* fileCopy_p, int fileSize, int dirOffset, int dirEntry);
int getFileSize(char* p, int offset, int dirEntryNum);
int logicalToPhysicalSector(int logicalSector);
void addFileToDirectory(char* p, char* fileName, int offset);
void addMetadataToDir(char* p, char* fileName, int size, int offset, uint8_t logicalSector);
void copyToDisk(char* p, char* localFile_p, char* fileName, int size, int offset);

#endif //CSC360_ASSIGNMENT3_DISKMETHODS_H

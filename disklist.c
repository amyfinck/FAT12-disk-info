#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include<sys/mman.h>
#include<string.h>
#include "linkedlist.h"

#define BYTES_PER_SECTOR 512
#define BYTES_PER_DIR_ENTRY 32

static unsigned int bytes_per_sector;
static char* os_name;
static char* disk_label;
static unsigned int sectors_per_cluster;
static unsigned int reserved_clusters;
static unsigned int fat_count;
static unsigned int max_root_entries;
static unsigned int total_sector_count;
static unsigned int sectors_per_fat;
static unsigned int num_heads;

void printDirectory(char* p, int offset, char* dirName)
{
    Subdirectory * head = NULL;

    printf("%s\n", dirName);
    printf("==================\n");
    int byteOffset = offset * BYTES_PER_SECTOR;
    for(int i = 0; i < 16 * BYTES_PER_DIR_ENTRY; i += BYTES_PER_DIR_ENTRY)
    {
        // if this entry is not empty
        if((p + byteOffset + i)[0] != 0x00)
        {
            // attributes offset is 11
            uint16_t attributes = 0;
            memcpy(&attributes, (p + byteOffset + i + 11), 1);

            // check that it isn't a volume label or a subdirectory, then its a file
            if( ((attributes & 0x08) == 0) && ((attributes & 0x10) == 0))
            {
                printf("F ");

                int fileSize = 0;
                memcpy(&fileSize, (p + byteOffset + i + 28), 4);
                printf("%10d ", fileSize);

                char* fullFileName = malloc(sizeof(char) * 13);
                char* fileName = malloc(sizeof(char) * 8);
                char* fileExt = malloc(sizeof(char) * 3);
                strncpy(fileName, (p + byteOffset + i + 0), 8);
                for(int j = 0; j < 8; j++) {
                    if(fileName[j] == ' ') fileName[j] = '\0';
                }
                strncpy(fileExt, (p + byteOffset + i + 8), 3);
                for(int k = 0; k < 3; k++) {
                    if(fileExt[k] == ' ') fileExt[k] = '\0';
                }
                snprintf(fullFileName, 13, "%s.%s", fileName, fileExt);
                printf("%20s \n", fullFileName);
                free(fileName);
                free(fileExt);
                free(fullFileName);

//                int creationDate = 0;
//                memcpy(&fileSize, (p + byteOffset + i + 28), 4);
//                printf("%10d ", fileSize);
//
//                int creationTime = 0;
//                memcpy(&fileSize, (p + byteOffset + i + 28), 4);
//                printf("%10d ", fileSize);
            }

            // if it is a subdirectory
            if((attributes & 0x10) != 0)
            {
                int dirSize = 0;
                memcpy(&dirSize, (p + byteOffset + i + 28), 4);

                char* subdirName = malloc(sizeof(char) * 8);
                strncpy(subdirName, (p + byteOffset + i + 0), 8);
                for(int j = 0; j < 8; j++) {
                    if(subdirName[j] == ' ') subdirName[j] = '\0';
                }

                if(subdirName[0] != '.')
                {
                    printf("D ");
                    printf("%10d ", dirSize);
                    printf("%20s \n", subdirName);

                    // if this is a subdirectory, find where it starts (first logical cluster field)
                    uint16_t firstLogicalCluster = 0;
                    memcpy(&firstLogicalCluster, (p + byteOffset + i + 26), 2);

                    // store in linked list until end of subdirectory
                    head = addSubdir(33 + firstLogicalCluster - 2, subdirName, head);
                }
            }
        }
    }
    while(head != NULL)
    {
        printDirectory(p, head->offset, head->subdirName);
        head = deleteSub(head->offset, head);
        // TODO fix segfault
        //free(head->subdirName);
    }
}

int main(int argc, char* argv[])
{
    if(argc == 1)
    {
        printf("Input image required\n");
        exit(1);
    }
    else if(argc > 2)
    {
        printf("Too many arguments\n");
        exit(1);
    }

    int file_descriptor;
    struct stat statbuf;

    // open is a lower-level version of fopen. Note in example they use O_RDWR, but I think I only want to read.
    file_descriptor = open(argv[1], O_RDWR);

    // fstat gets the file status, and puts all the relevant information in sb
    fstat(file_descriptor, &statbuf);

    // p points to starting position of mapped memory
    char * p = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);
    if (p == MAP_FAILED)
    {
        printf("Error: failed to map memory\n");
        exit(1);
    }

    // offset of 19 to start with root directory
    printDirectory(p, 19, "ROOT");

    // unmap region
    munmap(p, statbuf.st_size); // the modifed the memory data would be mapped to the disk image
    close(file_descriptor);

    free(os_name);
    free(disk_label);
    return 0;
}
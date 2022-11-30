#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include<sys/mman.h>
#include<string.h>
#include "diskmethods.h"

void copyFileFromRoot(char* p, char* fileName)
{
    // check that the file exists
    if(isFileInDirectory(p, ROOT_OFFSET, fileName) == -1)
    {
        printf("File not found\n");
        return;
    }

    // if it is found, copy it from the root
}

int main(int argc, char* argv[])
{
    if(argc == 2)
    {
        printf("Input image and file to copy required\n");
        exit(1);
    }
    else if(argc > 3)
    {
        printf("Too many arguments\n");
        exit(1);
    }

    int file_descriptor;
    struct stat statbuf;
    file_descriptor = open(argv[1], O_RDWR);
    fstat(file_descriptor, &statbuf);

    // p points to starting position of mapped memory
    char * p = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);
    if (p == MAP_FAILED)
    {
        printf("Error: failed to map memory\n");
        exit(1);
    }

    copyFileFromRoot(p, argv[2]);

    // unmap region
    munmap(p, statbuf.st_size); // the modifed the memory data would be mapped to the disk image
    close(file_descriptor);

    return 0;
}
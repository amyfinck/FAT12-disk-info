#include "diskmethods.h"

void addFileToDirectory(char* p, char* fileName, int offset)
{
    int fileDescriptor = open(fileName, O_RDWR);
    struct stat statbuf;
    if (fileDescriptor == -1)
    {
        printf("Error: file not found in current directory\n");
        exit(1);
    }
    fstat(fileDescriptor, &statbuf);

    // p points to starting position of mapped memory
    char * localFile_p = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
    if (localFile_p == MAP_FAILED)
    {
        printf("Error: failed to map memory\n");
        exit(1);
    }

    int freeSectors = getFreeSectorCount(p);

    if(statbuf.st_size > freeSectors * BYTES_PER_SECTOR)
    {
        printf("Error: input file is too large\n");
        exit(1);
    }

    copyToDisk(p, localFile_p, fileName, statbuf.st_size, offset);
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

    addFileToDirectory(p, argv[2], ROOT_OFFSET);

    // unmap region
    munmap(p, statbuf.st_size);
    close(file_descriptor);

    return 0;
}
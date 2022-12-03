#include "diskmethods.h"

void addFileToDirectory(char* p, char* fileName, int offset)
{

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

    int fileDesc_ima;
    struct stat statBuf_ima;
    fileDesc_ima = open(argv[1], O_RDWR);
    if(fileDesc_ima == -1)
    {
        close(fileDesc_ima);
        printf("Could not locate image in current directory\n");
        exit(1);
    }
    fstat(fileDesc_ima, &statBuf_ima);

    // p points to starting position of mapped memory
    char * p = mmap(NULL, statBuf_ima.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fileDesc_ima, 0);
    if (p == MAP_FAILED)
    {
        printf("Error: failed to map memory\n");
        close(fileDesc_ima);
        munmap(p, statBuf_ima.st_size);
        exit(1);
    }

    //------------------------

    int fileDesc_file = open(argv[2], O_RDWR);
    struct stat statBuf_file;
    if (fileDesc_file == -1)
    {
        printf("Error: file not found in current directory\n");
        munmap(p, statBuf_file.st_size);
        close(fileDesc_ima);
        close(fileDesc_file);
        exit(1);
    }
    fstat(fileDesc_file, &statBuf_file);

    // p points to starting position of mapped memory
    char * localFile_p = mmap(NULL, statBuf_file.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fileDesc_file, 0);
    if (localFile_p == MAP_FAILED)
    {
        printf("Error: failed to map memory 2\n");
        munmap(p, statBuf_file.st_size);
        munmap(localFile_p, statBuf_file.st_size);
        close(fileDesc_ima);
        close(fileDesc_file);
        exit(1);
    }

    int freeSectors = getFreeSectorCount(p);

    if(statBuf_file.st_size > freeSectors * BYTES_PER_SECTOR)
    {
        printf("Error: input file is too large\n");
        munmap(p, statBuf_file.st_size);
        munmap(localFile_p, statBuf_file.st_size);
        close(fileDesc_ima);
        close(fileDesc_file);
        exit(1);
    }

    copyToDisk(p, localFile_p, argv[2], statBuf_file.st_size, ROOT_OFFSET);

    munmap(p, statBuf_ima.st_size);
    munmap(localFile_p, statBuf_file.st_size);

    close(fileDesc_ima);
    close(fileDesc_file);
    return 0;
}
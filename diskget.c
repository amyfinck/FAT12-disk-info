#include "diskmethods.h"

void copyFileFromRoot(char* p, char* fileName)
{
    // check that the file exists
    int dirEntryNum = getFileDirEntry(p, ROOT_OFFSET, fileName);
    if(dirEntryNum == -1)
    {
        printf("File not found in root directory - ensure name and extention in all caps\n");
        return;
    }

    // create a file in the current directory with the same name
    int filedescriptor;
    struct stat statbuf;
    filedescriptor = open(fileName, O_RDWR | O_CREAT, 0666);
    fstat(filedescriptor, &statbuf);

    int fileSize = getFileSize(p, ROOT_OFFSET, dirEntryNum);

    // stretch the file to fileSize so we map the right sized file
    off_t lretval = lseek(filedescriptor, fileSize-1, SEEK_SET);
    if(lretval == -1) {
        printf("error: lseek failed\n");
        close(filedescriptor);
        exit(1);
    }

    ssize_t wretval = write(filedescriptor, "", 1);
    if(wretval == -1){
        printf("error: write failed\n");
        close(filedescriptor);
        exit(1);
    }

    char* fileCopy_p = mmap(NULL, fileSize, PROT_WRITE, MAP_SHARED, filedescriptor, 0);
    if (fileCopy_p == MAP_FAILED) {
        printf("Error: failed to map memory\n");
        exit(1);
    }

    // copy it from the root to the current directory
    copyFileToLocalDir(p, fileCopy_p, fileSize, ROOT_OFFSET, dirEntryNum);

    munmap(fileCopy_p, statbuf.st_size);

    printf("%s has been copied to your current directory\n", fileName);
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
        printf("Error: failed to map memory HERE\n");
        exit(1);
    }

    copyFileFromRoot(p, argv[2]);

    // unmap region
    munmap(p, statbuf.st_size);
    close(file_descriptor);

    return 0;
}
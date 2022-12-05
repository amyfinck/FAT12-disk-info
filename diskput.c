#include "diskmethods.h"

// extracts the file name from the fullFileName and copies it into fileNameRet
void getFileName(char* fullFileName, char* fileNameRet)
{
    char* fileName = malloc(sizeof(char)*13);
    const char s[2] = "/";
    char* token = strtok(fullFileName, s);

    strncpy(fileName, token, 13);
    while(token != NULL)
    {
        fileName = token;
        token = strtok(NULL, s);
    }
    strncpy(fileNameRet, fileName, 13);
}

char* getPathOnly(char* fullFileName, char* fileName)
{
    fullFileName += strlen(fullFileName) - strlen(fileName);

    for(int i = 0; i < strlen(fileName); i++)
    {
        fullFileName[i] = '\0';
    }
    return fullFileName;
}

int moveThroughFilePath(char*p, char* fullFileName, char* fileName)
{
    const char s[2] = "/";
    char* token = strtok(fullFileName, s);

    int offset = 19;
    while(token != NULL && strncmp(token, fileName, 13) != 0)
    {
        offset = isSubdirectory(p, token, offset);
        token = strtok(NULL, s);
    }
    return offset;
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

    char* fullFileName1 = malloc(sizeof(char)*100);
    char* fullFileName2 = malloc(sizeof(char)*100);
    char* fullFileName3 = malloc(sizeof(char)*100);
    char* fileName = malloc(sizeof(char)*8);

    strncpy(fullFileName1, argv[2], 100);
    strncpy(fullFileName2, argv[2], 100);
    strncpy(fullFileName3, argv[2], 100);

    getFileName(fullFileName1, fileName);

    int offset = moveThroughFilePath(p, fullFileName2, fileName);
    if(offset == -1)
    {
        printf("Specified path could not be found on image\n");
        close(fileDesc_ima);
        munmap(p, statBuf_ima.st_size);
        exit(1);
    }

    int fileDesc_file = open(fileName, O_RDWR);
    struct stat statBuf_file;
    if (fileDesc_file == -1)
    {
        printf("Error: file not found in current directory\n");
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

    copyToDisk(p, localFile_p, fileName, statBuf_file.st_size, offset);

    if(offset == 19)
    {
        printf("%s has been copied to root directory\n", fileName);
    }
    else
    {
        printf("%s has been copied to the specified directory\n", fileName);
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

    munmap(p, statBuf_ima.st_size);
    munmap(localFile_p, statBuf_file.st_size);

    free(fullFileName1);
    free(fullFileName2);
    free(fullFileName3);
    free(fileName);

    close(fileDesc_ima);
    close(fileDesc_file);
    return 0;
}
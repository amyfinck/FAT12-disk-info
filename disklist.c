#include "linkedlist.h"
#include "diskmethods.h"

static char* os_name;
static char* disk_label;

void printDirectory(char* p, int offset, char* dirName)
{
    // TODO - refactor so date, time, size only accessed once for files and subdirs

    int subdirOffsets[16] = {0};
    char subdirNames[16][8];

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

            // check that it isn't a volume label or a subdirectory, then it's a file
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
                printf("%20s ", fullFileName);
                free(fileName);
                free(fileExt);
                free(fullFileName);

                uint16_t creationDate = 0;
                memcpy(&creationDate, (p + byteOffset + i + 16), 2);
                uint16_t creationTime = 0;
                memcpy(&creationTime, (p + byteOffset + i + 14), 2);
                printCreationDateTime(creationDate, creationTime);
                printf("\n");
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
                    printf("%20s ", subdirName);

                    // if this is a subdirectory, find where it starts (first logical cluster field)
                    uint16_t firstLogicalCluster = 0;
                    memcpy(&firstLogicalCluster, (p + byteOffset + i + 26), 2);

                    // store to call recursively at end of function
                    for(int j = 0; j < 16; j++)
                    {
                        if(subdirOffsets[j] == 0)
                        {
                            subdirOffsets[j] = 33 + firstLogicalCluster - 2;
                            for(int k = 0; k < 8; k++)
                            {
                                subdirNames[j][k] = subdirName[k];
                            }
                            break;
                        }
                    }

                    // TODO - Where is this located for subdirectories? These bits appear to all be 0.
                    uint16_t creationDate = 0;
                    memcpy(&creationDate, (p + byteOffset + i + 16), 2);
                    uint16_t creationTime = 0;
                    memcpy(&creationTime, (p + byteOffset + i + 14), 2);
                    printCreationDateTime(creationDate, creationTime);
                    printf("\n");
                }
            }
        }
    }
    for(int i = 0; i < 16; i++)
    {
        if(subdirOffsets[i] == 0) break;
        printDirectory(p, subdirOffsets[i], subdirNames[i]);
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

    file_descriptor = open(argv[1], O_RDWR);
    // TODO - check on fail to open file

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
    printDirectory(p, ROOT_OFFSET, "ROOT");

    // unmap region
    munmap(p, statbuf.st_size); // the modifed the memory data would be mapped to the disk image
    close(file_descriptor);

    free(os_name);
    free(disk_label);
    return 0;
}
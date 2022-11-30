#include "diskmethods.h"

// gets the fat table entry at position, starting at 0 for the first entry.
int getFatEntry(char* p, int position)
{
    // skip boot sector
    int fatOffset = BYTES_PER_SECTOR;
    uint16_t fatEntry;

    if(position % 2 == 0)
    {
        int secondByteOffset = (3 * position) / 2;
        int firstByteOffset = secondByteOffset + 1;

        uint16_t firstByte = 0;
        uint16_t secondByte = 0;

        memcpy(&firstByte, (p + 1*BYTES_PER_SECTOR + firstByteOffset), 1);
        memcpy(&secondByte, (p + 1*BYTES_PER_SECTOR + secondByteOffset), 1);

        fatEntry = (firstByte & 0xF) << 8 | secondByte;
    }
    else
    {
        int firstByteOffset = (3 * position) / 2;
        int secondByteOffset = firstByteOffset + 1;

        uint8_t firstByte;
        uint8_t secondByte;

        memcpy(&firstByte, (p + fatOffset + firstByteOffset), 1);
        memcpy(&secondByte, (p + fatOffset + secondByteOffset), 1);

        fatEntry = secondByte << 4 | (firstByte & 0xF0) >> 4;
    }
    return fatEntry;
}

int getFreeSectorCount(char* p)
{
    int freeSectors = 0;
    // for all sectors in the data area
    for(int i = 2; i <= 2848; i++)
    {
        if(getFatEntry(p, i) == 0x00) freeSectors++;
    }
    return freeSectors;
}

int getFilesOnDisk(char* p, int offset)
{
    int fileCount = 0;
    int byteOffset = offset * BYTES_PER_SECTOR;
    for(int i = 0; i < 16 * BYTES_PER_DIR_ENTRY; i += BYTES_PER_DIR_ENTRY)
    {
        // if this entry is not empty
        if((p + byteOffset + i)[0] != 0x00)
        {
            // attributes offset is 11
            uint16_t attributes = 0;
            memcpy(&attributes, (p + byteOffset + i + 11), 1);

            // check that it isn't a volume label or a subdirectory
            if( ((attributes & 0x08) == 0) && ((attributes & 0x10) == 0))
            {
                fileCount++;
            }

            // if it is a subdirectory
            if((attributes & 0x10) != 0)
            {
                char* subdirName = malloc(sizeof(char) * 8);
                strncpy(subdirName, (p + byteOffset + i + 0), 8);
                if(subdirName[0] != '.')
                {
                    uint16_t firstLogicalCluster;
                    memcpy(&firstLogicalCluster, (p + byteOffset + i + 26), 2);
                    // recursively call this method for this subdirectory
                    fileCount += getFilesOnDisk(p, 33 + firstLogicalCluster - 2);
                }
                free(subdirName);
            }
        }
    }
    return fileCount;
}

void printCreationDateTime(int creationDate, int creationTime)
{
    /*
     * creationDate info from wikipedia:
     * bits 15 - 9 are the year (0 = 1980)
     * bits 8 - 5 are month (1 - 12)
     * bits 4 - 0 are day
     */

    uint8_t year = (creationDate & 0xFE00) >> 9;
    uint8_t month = (creationDate & 0x01E0) >> 5;
    uint8_t day = (creationDate & 0x001F);

    printMonth(month);
    printf(" ");
    if(day != 0) printf("%2d ", day);
    if(year != 0) printf("%d ", year + 1980);

    /*
     * creationTime info from wikipedia:
     * bits 15 - 11 are hours (0–23)
     * bits 10 - 5 are minutes (0-59)
     * bits 4 - 0 are seconds/2 (0-29) (ignore)
     */
    uint8_t hours = (creationTime & 0xF800) >> 11;
    uint8_t minutes = (creationTime & 0x07E0) >> 5;

    if(!(hours == 0 & minutes == 0)) printf("%2d:%02d", hours, minutes);
}

void printMonth(int month)
{
    switch(month)
    {
        case 1:
            printf("Jan");
            break;
        case 2:
            printf("Feb");
            break;
        case 3:
            printf("Mar");
            break;
        case 4:
            printf("Apr");
            break;
        case 5:
            printf("May");
            break;
        case 6:
            printf("Jun");
            break;
        case 7:
            printf("Jul");
            break;
        case 8:
            printf("Aug");
            break;
        case 9:
            printf("Sep");
            break;
        case 10:
            printf("Oct");
            break;
        case 11:
            printf("Nov");
            break;
        case 12:
            printf("Dec");
            break;
        default:
            printf("   ");
    }
}

// returns the directory entry number that fileName exists at, if not found returns -1.
int getFileDirEntry(char* p, int offset, char* fileName)
{
    int byteOffset = offset * BYTES_PER_SECTOR;

    // for each entry in the directory
    for(int i = 0; i < 16 * BYTES_PER_DIR_ENTRY; i += BYTES_PER_DIR_ENTRY)
    {
        // if this entry is not empty
        if((p + byteOffset + i)[0] != 0x00)
        {
            uint16_t attributes = 0;
            memcpy(&attributes, (p + byteOffset + i + 11), 1); // attributes offset is 11

            // check that it isn't a volume label or a subdirectory, then it's a file
            if( ((attributes & 0x08) == 0) && ((attributes & 0x10) == 0))
            {
                char* fullFileName = malloc(sizeof(char) * 13);
                char* dirFileName = malloc(sizeof(char) * 8);
                char* fileExt = malloc(sizeof(char) * 3);
                strncpy(dirFileName, (p + byteOffset + i + 0), 8);
                for(int j = 0; j < 8; j++) {
                    if(dirFileName[j] == ' ') dirFileName[j] = '\0';
                }
                strncpy(fileExt, (p + byteOffset + i + 8), 3);
                for(int k = 0; k < 3; k++) {
                    if(fileExt[k] == ' ') fileExt[k] = '\0';
                }
                snprintf(fullFileName, 13, "%s.%s", dirFileName, fileExt);

                if(strncmp(fullFileName, fileName, 13) == 0)
                {
                    return i / BYTES_PER_DIR_ENTRY;
                }

                free(dirFileName);
                free(fileExt);
                free(fullFileName);
            }
        }
    }
    return -1;
}

int getFileSize(char* p, int offset, int dirEntryNum)
{
    int fileSize = 0;
    memcpy(&fileSize, (p + offset*BYTES_PER_SECTOR + dirEntryNum*BYTES_PER_DIR_ENTRY + 28), 4);
    return fileSize;
}

void copyFileToLocalDir(char* p, char* fileCopy_p, int fileSize, int dirOffset, int dirEntry)
{
    int bytesToCopy = fileSize;

    uint16_t firstLogicalCluster;
    memcpy(&firstLogicalCluster, (p + dirOffset*BYTES_PER_SECTOR + dirEntry*BYTES_PER_DIR_ENTRY + 26), 2);

    int physicalSectorNumber = 33 + firstLogicalCluster - 2;

    int nextLogicalCluster = firstLogicalCluster;
    while((0x000 < nextLogicalCluster) && (nextLogicalCluster < 0xFF0))
    {
        for(int i = 0; i < BYTES_PER_SECTOR; i++)
        {
            if(bytesToCopy > 0)
            {
                fileCopy_p[fileSize - bytesToCopy] = p[logicalToPhysicalSector(nextLogicalCluster)* BYTES_PER_SECTOR + i];
                bytesToCopy--;
            }
        }
        nextLogicalCluster = getFatEntry(p, nextLogicalCluster);
    }
}

int logicalToPhysicalSector(int logicalSector)
{
    return 33 + logicalSector - 2;
}
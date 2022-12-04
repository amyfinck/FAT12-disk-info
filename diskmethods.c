#include "diskmethods.h"

int getDiskSize(char* p)
{
    int total_sector_count;
    memcpy(&total_sector_count, (p + 19), 2);
    return total_sector_count * BYTES_PER_SECTOR;
}

int isSubdirectory(char* p, char* dirName, int offset)
{
    // skip forward to the directory, offset is in sectors
    char* temp_ptr = p;
    temp_ptr += BYTES_PER_SECTOR * offset;

    int fileCount = 0;
    int i = 0;

    // 0x00 entry means all others are empty, 16*14 sectors total
    while ((i < 16*14) && (temp_ptr[0] != 0x00))
    {
        if( !((temp_ptr[11] & 0b00010000) == 0) )
        {
            // get subdirectory name
            char* subdirName = malloc(sizeof(char) * 8);
            strncpy(subdirName, (temp_ptr + 0), 8);
            for(int j = 0; j < 8; j++) {
                if(subdirName[j] == ' ') subdirName[j] = '\0';
            }

            uint16_t firstLogicalCluster;
            memcpy(&firstLogicalCluster, (temp_ptr + 26), 2);
            if(strncmp(subdirName, dirName, 8) == 0)
            {
                return firstLogicalCluster + 31;
            }
            else if(subdirName[0] != '.')
            {
                //return isSubdirectory(p, dirName, firstLogicalCluster + 31);
            }
            free(subdirName);
        }
        // skip to next dir entry
        temp_ptr += 32;
        i++;
    }
    return -1;
}


// gets the fat table entry at position, starting at 3 for first entry
// TODO - change position to logical sector for consistency
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

void writeToFat(char *p, int logSect, int value)
{
    //printf("writing %d into %d\n", value, logSect);
    // Skip boot sector and go to first FAT
    p += BYTES_PER_SECTOR;

    if (logSect % 2 == 0)
    {
        p[(3*logSect)/2+1] = ((value >> 8) & 0x0F) | ( p[1+(3*logSect)/2] & 0xF0);
        p[(3*logSect)/2] = value & 0xFF;
    }
    else
    {
        p[(3*logSect)/2] = ((value<<4) & 0xF0) | (p[(3*logSect)/2] & 0x0F);
        p[1+(3*logSect)/2] = (value>>4) & 0xFF;
    }
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

int getFreeSector(char* p)
{
    for(int i = 2; i <= 2848; i++)
    {
        if(getFatEntry(p, i) == 0x00) return i;
    }
    return -1;
}

int getFilesOnDisk(char* p, int offset)
{
    // skip forward to the directory, offset is in sectors
    char* temp_ptr = p;
    temp_ptr += BYTES_PER_SECTOR * offset;

    int fileCount = 0;
    int i = 0;

    // 0x00 entry means all others are empty, 16*14 sectors total
    while ((i < 16*14) && (temp_ptr[0] != 0x00))
    {
        // not hidden, volume label, or subdirectory
        if ((temp_ptr[11] & 0b00000010) == 0 && (temp_ptr[11] & 0b00001000) == 0 && (temp_ptr[11] & 0b00010000) == 0)
        {
            //char* subdirName = malloc(sizeof(char) * 8);
            //strncpy(subdirName, (temp_ptr + 0), 8);
            fileCount++;
            //free(subdirName);
        }
        else if( !((temp_ptr[11] & 0b00010000) == 0) )
        {
            // it is a subdirectory
            char* subdirName = malloc(sizeof(char) * 8);
            strncpy(subdirName, (temp_ptr + 0), 8);
            if(subdirName[0] != '.')
            {
                uint16_t firstLogicalCluster;
                memcpy(&firstLogicalCluster, (temp_ptr + 26), 2);
                // recursively call this method for this subdirectory
                if(firstLogicalCluster <= 2849)
                {
                    fileCount += getFilesOnDisk(p, 33 + firstLogicalCluster - 2);
                }
            }
            free(subdirName);
        }
        // skip to next dir entry
        temp_ptr += 32;
        i++;
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

    // for each entry in the directory - there are 14 sectors in root directory, each with 16 entries
    for(int i = 0; i < 14 * 16 * BYTES_PER_SECTOR; i += BYTES_PER_DIR_ENTRY)
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

int getFreeDirEntry(char* p, int offset)
{
    int byteOffset = offset * BYTES_PER_SECTOR;

    // for each entry in the directory
    for(int i = 0; i < 14*16*BYTES_PER_DIR_ENTRY; i += BYTES_PER_DIR_ENTRY)
    {
        // if this entry is not empty
        if((p + byteOffset + i)[0] == 0x00)
        {
            return i / BYTES_PER_DIR_ENTRY;
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
        //printf("%d ", nextLogicalCluster);
        nextLogicalCluster = getFatEntry(p, nextLogicalCluster);
    }
}

int logicalToPhysicalSector(int logicalSector)
{
    return 33 + logicalSector - 2;
}

void addMetadataToDir(char* p, char* fullFileName, int size, int offset, uint16_t logicalSector)
{
    int freeEntry = getFreeDirEntry(p, offset);
    if(freeEntry == -1)
    {
        printf("Error: Directory is full");
        exit(1);
    }

    char* fullFileNameCopy = malloc(sizeof(char) * 13);
    char* fileName = malloc(sizeof(char) * 8);
    char* fileExt = malloc(sizeof(char) * 3);

    char *delim = ".";
    strncpy(fullFileNameCopy, fullFileName, 13);
    strncpy(fileName, strtok(fullFileNameCopy, delim), 8);
    strncpy(fileExt, strtok(NULL, delim), 3);

    for(int i = 0; i < 8; i++)
    {
        // file name
        p[offset*BYTES_PER_SECTOR + freeEntry*BYTES_PER_DIR_ENTRY + i] = toupper(fileName[i]);
    }
    for(int i = 0; i < 3; i++)
    {
        // file ext
        p[offset*BYTES_PER_SECTOR + freeEntry*BYTES_PER_DIR_ENTRY + 8 + i] = toupper(fileExt[i]);
    }

    // attribute
    p[offset*BYTES_PER_SECTOR + freeEntry*BYTES_PER_DIR_ENTRY + 11] = 0x00;

    time_t time_now = time(NULL);
    struct tm *now = localtime(&time_now);
    int year = now->tm_year + 1900;
    int month = now->tm_mon + 1;
    int day = now->tm_mday;
    uint8_t hour = now->tm_hour;
    uint8_t minute = now->tm_min;

    // TODO - do I need to do lastwritetime, ect
    p[offset*BYTES_PER_SECTOR + freeEntry*BYTES_PER_DIR_ENTRY + 14] = ((minute & 0x3F) << 5 | 0x00);
    p[offset*BYTES_PER_SECTOR + freeEntry*BYTES_PER_DIR_ENTRY + 15] = ((hour & 0x1F) << 3 | (minute & 0x3F) >> 3);
    p[offset*BYTES_PER_SECTOR + freeEntry*BYTES_PER_DIR_ENTRY + 16] = ( (month & 0xF) << 5 | (day & 0x1F) );
    p[offset*BYTES_PER_SECTOR + freeEntry*BYTES_PER_DIR_ENTRY + 17] = (( (year-1980) & 0x7F) << 1 | (month & 0xF) >> 3);

    p[offset*BYTES_PER_SECTOR + freeEntry*BYTES_PER_DIR_ENTRY + 26] = (logicalSector & 0xFF);
    p[offset*BYTES_PER_SECTOR + freeEntry*BYTES_PER_DIR_ENTRY + 27] = (logicalSector & 0xFF00) >> 8;

    p[offset*BYTES_PER_SECTOR + freeEntry*BYTES_PER_DIR_ENTRY + 28] = size & 0x000000FF;
    p[offset*BYTES_PER_SECTOR + freeEntry*BYTES_PER_DIR_ENTRY + 29] = (size & 0x0000FF00) >> 8;
    p[offset*BYTES_PER_SECTOR + freeEntry*BYTES_PER_DIR_ENTRY + 30] = (size & 0x00FF0000) >> 16;
    p[offset*BYTES_PER_SECTOR + freeEntry*BYTES_PER_DIR_ENTRY + 31] = (size & 0xFF000000) >> 24;

    free(fullFileNameCopy);
    free(fileName);
    free(fileExt);
}

void copyToDisk(char* p, char* localFile_p, char* fileName, int size, int offset)
{
    int bytesToCopy = size;

    int freeSector = getFreeSector(p);
    int prevSector = -1;

    if(freeSector == -1)
    {
        printf("Error: No free sectors remain\n");
        exit(1);
    }

    addMetadataToDir(p, fileName, size, offset, freeSector);

    for(int j = 0; j <= size / BYTES_PER_SECTOR; j++)
    {
        for(int i = 0; i < BYTES_PER_SECTOR; i++)
        {
            if (bytesToCopy > 0)
            {
                p[logicalToPhysicalSector(freeSector) * BYTES_PER_SECTOR + i] = localFile_p[size - bytesToCopy];
                bytesToCopy--;
            }
        }
        writeToFat(p, freeSector, 0xFFF);
        int nextSector = getFreeSector(p);
        writeToFat(p, freeSector, nextSector);
        prevSector = freeSector;
        freeSector = nextSector;
    }
    if(prevSector != -1) writeToFat(p, prevSector, 0xFFF);
    writeToFat(p, freeSector, 0x000);
}
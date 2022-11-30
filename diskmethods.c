#include "diskmethods.h"

// gets the fat table entry at position, starting at 0 for the first entry.
int getFatEntry(int position, char* p)
{
    // 512 to skip boot sector, first 2 entries are reserved, each entry is 12 bits, so 2 entries is 3 bytes
    int fatOffset = 512 + 3;
    uint16_t fatEntry;

    if(position % 2 == 0)
    {
        int secondByteOffset = (3 * position) / 2;
        int firstByteOffset = secondByteOffset + 1;

        uint16_t firstByte = 0;
        uint16_t secondByte = 0;

        memcpy(&firstByte, (p + fatOffset + firstByteOffset), 1);
        memcpy(&secondByte, (p + fatOffset + secondByteOffset), 1);

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
    for(int i = 0; i <= 2846; i++)
    {
        if(getFatEntry(i, p) == 0x00) freeSectors++;
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
            // TODO - TEST IF THIS WORKS
            if((attributes & 0x10) != 0)
            {
                // if this is a subdirectory, find where it starts (first logical cluster field)
                uint16_t firstLogicalCluster = 0;
                memcpy(&firstLogicalCluster, (p + byteOffset + i + 26), 2);

                fileCount += getFilesOnDisk(p, 33 + firstLogicalCluster);
            }
        }
    }
    return fileCount;
}

void printCreationDateTime(int creationDate, int creationTime)
{

    /*
     * creationDate info from wikipedia
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
     * creationTime info from wikipedia
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


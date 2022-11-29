#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include<sys/mman.h>
#include<string.h>

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

void getVolumeLabel(char* p)
{
    // if we get a value for disk label in the boot
    if(disk_label[0] != ' ') return;

    // otherwise look for an entry in the root directory with attribute "volume_label" (0x08)
    int rootOffset = 19 * BYTES_PER_SECTOR;
    for(int i = 0; i < 16 * BYTES_PER_DIR_ENTRY; i += BYTES_PER_DIR_ENTRY)
    {
        // attributes offset is 11
        uint16_t attributes = 0;
        memcpy(&attributes, (p + rootOffset + i + 11), 1);

        if(attributes == 0x08)
        {
            strncpy(disk_label, p + rootOffset + i, 11);
            return;
        }
    }
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

    // boot sector
    os_name = malloc(sizeof(char) * 8);
    if(os_name == NULL) printf("error - malloc failed\n");
    disk_label = malloc(sizeof(char) * 11);
    if(disk_label == NULL) printf("error - malloc failed\n");
    // TODO - remove unused ones
    strncpy(os_name, (p + 3), 8);
    strncpy(disk_label, (p + 43), 11);
    memcpy(&bytes_per_sector, (p + 11), 2);
    memcpy(&reserved_clusters, (p + 14), 2);
    memcpy(&fat_count, (p + 16), 1);
    memcpy(&total_sector_count, (p + 19), 2);
    memcpy(&sectors_per_fat, (p + 22), 2);

    printf("OS Name: %s\n", os_name);
    getVolumeLabel(p);
    printf("Label of the disk: %s\n", disk_label);
    printf("Total size of the disk: %d bytes\n", total_sector_count * bytes_per_sector);
    printf("Free size of the disk: %d bytes\n", getFreeSectorCount(p) * bytes_per_sector);
    printf("\n");
    printf("==============\n");

    // offset parameter = 19 for root directory
    printf("The number of files in the disk: %d\n\n", getFilesOnDisk(p, 19));
    printf("Number of FAT copies: %d\n", fat_count);
    printf("Sectors per FAT: %d\n", sectors_per_fat);

    // unmap region
    munmap(p, statbuf.st_size); // the modifed the memory data would be mapped to the disk image
    close(file_descriptor);

    free(os_name);
    free(disk_label);
    return 0;
}

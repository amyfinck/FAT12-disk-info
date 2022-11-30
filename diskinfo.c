#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include<sys/mman.h>
#include<string.h>
#include "diskmethods.h"

#define BYTES_PER_SECTOR 512
#define BYTES_PER_DIR_ENTRY 32
#define ROOT_OFFSET 19

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
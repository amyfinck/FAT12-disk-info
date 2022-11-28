#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include<sys/mman.h>
#include<string.h>

static unsigned int bytes_per_sector;
static char* os_name;
static unsigned int sectors_per_cluster;
static unsigned int reserved_clusters;
static unsigned int fat_count;
static unsigned int max_root_entries;
static unsigned int total_sector_count;
static unsigned int sectors_per_fat;

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

    /*
     * void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
     *      - addr is NULL
     *          - If addr is NULL, then the kernel chooses the (page-aligned) address at which to create the mapping
     *      - the length is the number of bytes, so for sample it is 1474560
     *      - prot means memory protection
     *          - PROT_READ: Pages may be read
     *          - PROT_WRITE: Pages may be written
     *      - flags
     *          - MAP_SHARED: Share this mapping.  Updates to the mapping are visible to other processes mapping the
     *            same region, and (in the case of file-backed mappings) are carried through to the underlying file.
     *      - int fd
     *          - This is our file descriptor
     *      - offset 0
     *          - no offset
     * On success, mmap() returns a pointer to the mapped area.
     */
    char * p = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0); // p points to the starting pos of your mapped memory
    if (p == MAP_FAILED)
    {
        printf("Error: failed to map memory\n");
        exit(1);
    }

    // boot sector
    os_name = malloc(sizeof(char) * 13);
    if(os_name == NULL) printf("error - malloc failed\n");

    strncpy(os_name, (p + 3), 13);
    memcpy(&bytes_per_sector, (p + 11), 2);
    memcpy(&sectors_per_cluster, (p + 13), 1);
    memcpy(&reserved_clusters, (p + 14), 2);
    memcpy(&fat_count, (p+ 16), 1);
    memcpy(&max_root_entries, (p+ 17), 2);
    memcpy(&total_sector_count, (p+ 19), 2);
    memcpy(&sectors_per_fat, (p+ 19), 2);


    printf("OS Name: %s\n", os_name);
    //printf("Label of the disk: %s\n", )
    //printf("Total size of the disk: %d\n");
    //printf("Free size of the disk: %d\n");
    //printf("\n")
    printf("==============\n");
    //printf("The number of files in the disk: %d\n\n");
    printf("Number of FAT copies: %d\n", fat_count);
    printf("Sectors per FAT: %d\n", sectors_per_fat);



//    printf("there are %d bytes per sector in this mapping\n", bytes_per_sector);
//    printf("there are %d sectors per cluster\n", sectors_per_cluster);
//    printf("there are %d reserved clusters\n", reserved_clusters);
//    printf("the fat count is %d\n", fat_count);
//    printf("max root entries is %d\n", max_root_entries);
//    printf("there are %d sectors total\n", total_sector_count);
//    printf("there are %d sectors per fat\n", sectors_per_fat);

    // unmap region
    munmap(p, statbuf.st_size); // the modifed the memory data would be mapped to the disk image
    close(file_descriptor);
    return 0;
}

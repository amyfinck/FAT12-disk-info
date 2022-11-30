#include "diskmethods.h"

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

    // unmap region
    munmap(p, statbuf.st_size);
    close(file_descriptor);

    return 0;
}
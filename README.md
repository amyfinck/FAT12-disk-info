# CSC360 Assignment 3
To compile, run `make`

### diskinfo
`./diskinfo <disk image>`

Prints:
* OS name
* disk label
* total size of disk
* free size of disk
* number of files on disk
* number of FAT copies
* sectors per FAT

### disklist
`./disklist <disk image>`

Starting from the root directory, prints the contents of each directory
on the disk. Each line represents a file or subdirectory, as follows:
* `D` to notate a directory, `S` to notate a subdirectory
* file size in bytes
* file or directory name
* file creation time and date

### diskget
`./diskget <disk image> <file name>`

`diskget` only retrieves files from the root directory of the disk,
use `disklist` to ensure the file is located there. Since FAT12
is case-insensitive, all filenames use uppercase letters. If a 
filename that is not all uppercase is passed in, it will be
converted to use all uppercase and that filename will be checked.

### diskput
`./diskput <disk image> <file path>`

`diskput` takes a file from the current directory and copies it
into the specified directory. If no file path is specified, it 
defaults to the root directory. For example

`./disput disk.IMA FILE.TXT`

would copy `FILE.TXT` into the root directory. 

`./diskput disk.IMA /SUB1/SUBSUB1/FILE.TXT`

would copy `FILE.TXT` into the subdirectory `SUBSUB1` located in
`SUB1`, which is itself located in the root directory, provided 
these subdirectories exist.

If any lowercase directories or filenames are passed in they will be
converted to uppercase.
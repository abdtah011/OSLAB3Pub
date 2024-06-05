#include <iostream>
#include <cstdint>
#include "disk.h"

#ifndef __FS_H__
#define __FS_H__

#define ROOT_BLOCK 0
#define FAT_BLOCK 1
#define FAT_FREE 0
#define FAT_EOF -1

#define TYPE_FILE 0
#define TYPE_DIR 1
#define READ 0x04
#define WRITE 0x02
#define EXECUTE 0x01

// DIR_SIZE calculates the number of directory entries that can fit into a single disk block.
// It does this by dividing the block size (BLOCK_SIZE) by the size of one directory entry (sizeof(dir_entry)).
#define DIR_SIZE BLOCK_SIZE/sizeof(dir_entry)
// FAT_ENTRIES calculates the number of FAT (File Allocation Table) entries that can fit into a single disk block.
// Since each FAT entry is 2 bytes, it divides the block size (BLOCK_SIZE) by 2 to determine the total number of FAT entries per block.
#define FAT_ENTRIES BLOCK_SIZE/2

struct dir_entry {
    char file_name[56]; // name of the file / sub-directory
    uint32_t size; // size of the file in bytes
    uint16_t first_blk; // index in the FAT for the first block of the file
    uint8_t type; // directory (1) or file (0)
    uint8_t access_rights; // read (0x04), write (0x02), execute (0x01)
};

struct cwd_struct {
    dir_entry enteries [DIR_SIZE];
    dir_entry informatioin; 
    int block; 
}; 


class FS {
private:
    Disk disk;
    // size of a FAT entry is 2 bytes
    int16_t fat[BLOCK_SIZE/2];

    cwd_struct cwd;

    // Helper function to find the directory entry for a given file
    dir_entry* find_dir_entry(const std::string& filename, dir_entry* root_dir);

public:
    FS();
    ~FS();
    // formats the disk, i.e., creates an empty file system
    int format();
    // create <filepath> creates a new file on the disk, the data content is
    // written on the following rows (ended with an empty row)
    int create(std::string filepath);
    /*######## help funktion for create ##############*/
    void file_parts(std::string filepath, std::string *filename, std::string *directury_path);
    dir_entry* find_directury_entry(std::string filname);
    int fin_a_empty_block(); 

    // cat <filepath> reads the content of a file and prints it on the screen
    int cat(std::string filepath);
    // ls lists the content in the current directory (files and sub-directories)
    int ls();

    // cp <sourcepath> <destpath> makes an exact copy of the file
    // <sourcepath> to a new file <destpath>
    int cp(std::string sourcepath, std::string destpath);
    // mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
    // or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
    int mv(std::string sourcepath, std::string destpath);
    // rm <filepath> removes / deletes the file <filepath>
    int rm(std::string filepath);
    // append <filepath1> <filepath2> appends the contents of file <filepath1> to
    // the end of file <filepath2>. The file <filepath1> is unchanged.
    int append(std::string filepath1, std::string filepath2);

    // mkdir <dirpath> creates a new sub-directory with the name <dirpath>
    // in the current directory
    int mkdir(std::string dirpath);
    // cd <dirpath> changes the current (working) directory to the directory named <dirpath>
    int cd(std::string dirpath);
    // pwd prints the full path, i.e., from the root directory, to the current
    // directory, including the current directory name
    int pwd();

    // chmod <accessrights> <filepath> changes the access rights for the
    // file <filepath> to <accessrights>.
    int chmod(std::string accessrights, std::string filepath);
};

#endif // __FS_H__

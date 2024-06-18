#include <iostream>
#include <cstring>
#include "fs.h"

FS::FS()
{
    std::cout << "FS::FS()... Creating file system\n";
}

FS::~FS()
{

}



// formats the disk, i.e., creates an empty file system
int
FS::format()
{ //KASPER
    std::cout << "FS::format()\n";
    // Initialize the FAT array to mark all entries as free
    std::fill(std::begin(fat), std::end(fat), FAT_FREE);

    //Mark reserved blocks
    fat[ROOT_BLOCK] = FAT_EOF;
    fat[FAT_BLOCK] = FAT_EOF;

    // Write the FAT to disk, to keep it ant PC shutdown. 
    if (disk.write(FAT_BLOCK, reinterpret_cast<uint8_t*>(fat)) != 0){
        std::cerr << "Error: Failed to write FAT to disk\n";
        return -1;
    }
    // Initialize the root directory block in memory
    // creating array and fill it with 0
    uint8_t root_block[BLOCK_SIZE] = {0};
    // Write the initialized root directory block to the disk
    if(disk.write(ROOT_BLOCK, root_block) != 0){
        std::cerr << "Error: Failed to write root directory block to disk \n";
        return -1;
    }
    
    return 0;
}

// create <filepath> creates a new file on the disk, the data content is
// written on the following rows (ended with an empty row)
int
FS::create(std::string filepath)
{
    std::cout << "FS::create(" << filepath << ")\n";
    std::string filename;
    std::string directury_path;

    // Get the parts of the filename 
    file_parts(filepath,&filename,&directury_path); 

    //check the file name and the directury to not be the same
    if (filename.empty()){

        std::cout << "Cannot create file with same name as a directory." << std::endl;
        return -1;
    }

    dir_entry* a_existing_file = find_directury_entry(filename);
    if (a_existing_file != nullptr){
        
        std::cout << "A file or directory with the "<< filename << " already exists" << std::endl;
  
    }
    

    // Collect data from the user
    std::string user_data; 
    std::string each_line; 
    while ( std::getline(std::cin , each_line))
    {
        if (each_line.empty()){
            break;
        }
        user_data.append(each_line + '\n'); 
    }
    

    // Create the file entry 

    dir_entry file; 
    int size = user_data.length() + 1; 
    strncmp(file.file_name, filename.c_str(), 56);
    file.size = size; 
    file.type = TYPE_FILE; 
    file.access_rights = READ | WRITE; 

    //Find a empty block 

    // int block_number = fin_a_empty_block();
    // if (block_number == -1){
        
    // }
    
    return 0;
}
//KASPER
// cat <filepath> reads the content of a file and prints it on the screen
int
FS::cat(std::string filepath)
{
    std::cout << "FS::cat(" << filepath << ")\n";

    //read the root directory block
    uint8_t root_block[BLOCK_SIZE];
    if(disk.read(ROOT_BLOCK, root_block) != 0){
        std::cerr <<"Error: Failed to read root directory block\n";
        return -1;
    }

    //converts the pointer type from uint8_t* to dir_entry*.
    dir_entry* root_dir = reinterpret_cast<dir_entry*>(root_block);
    
    //Finds the directory entry for the file

    dir_entry* file_entry = find_dir_entry(filepath, root_dir);
    if(!file_entry) {
        std::cerr << "Error: file not found\n";
        return -1;
    }

    // Check if the directory entry is a file
    if(file_entry->type != TYPE_FILE){
        std::cerr << "Error" << filepath << " is not a file \n";
        return -1;
    }

    //Read and print the file's data
    uint16_t blk = file_entry->first_blk;
    uint32_t size = file_entry->size;
    uint8_t buffer[BLOCK_SIZE];


    while (blk != FAT_EOF) { //////////////////////////////////////////////////////////////////////////////////////// check type uint16
        if (disk.read(blk,buffer) != 0) {
            std::cerr << "Error : Failed to read block " << blk << "\n";
            return -1;
        }

        //calculate how much to print from this block
        uint32_t to_print = std::min(size, static_cast<uint32_t>(BLOCK_SIZE));
        std::cout.write(reinterpret_cast<char*>(buffer), to_print);
        size -= to_print;
        blk = fat[blk];
    }

    std::cout << std::endl;
    return 0;
}


//KASPER
// ls lists the content in the currect directory (files and sub-directories)
int
FS::ls()
{
    std::cout << "FS::ls()\n";

     //read the root directory block
    uint8_t root_block[BLOCK_SIZE];
    if(disk.read(ROOT_BLOCK, root_block) != 0){
        std::cerr <<"Error: Failed to read root directory block\n";
        return -1;
    }

    //converts the pointer type from uint8_t* to dir_entry*.
    dir_entry* root_dir = reinterpret_cast<dir_entry*>(root_block);

    std::cout << "name:\n";
    for (int i = 0; i < BLOCK_SIZE / sizeof(dir_entry); i++){
        dir_entry& entry = root_dir[i];
        if (entry.file_name[0] != '\0') { // check if entry is valid (not empty)
            std::cout << entry.file_name << "\n";
        }
    }

    std::cout << "size\n";
    for (int i = 0; i < BLOCK_SIZE / sizeof(dir_entry); i++){
        dir_entry& entry = root_dir[i];
        if (entry.file_name[0] != '\0') { // check if entry is valid (not empty)
            std::cout << entry.size << "\n";
        }
    }    
    return 0;
}


//KASPER
// cp <sourcepath> <destpath> makes an exact copy of the file
// <sourcepath> to a new file <destpath>
int
FS::cp(std::string sourcepath, std::string destpath)
{
    std::cout << "FS::cp(" << sourcepath << "," << destpath << ")\n";

    // Read the root directory block
    uint8_t root_block[BLOCK_SIZE];
    if (disk.read(ROOT_BLOCK, root_block) != 0){
        std::cerr << "Error: Failed to read root directory block \n";
        return -1;
    }

    //converts the pointer type from uint8_t* to dir_entry*.
    dir_entry* root_dir = reinterpret_cast<dir_entry*>(root_block);

    //Find the source file
    dir_entry* src_entry = find_dir_entry(sourcepath, root_dir);
    if(!src_entry) {
        std::cerr << "Error: Source file not found \n";
        return -1;
    }

    //Ensure the source is a file
    if(src_entry->type != TYPE_FILE){
        std::cerr << "Error: " << sourcepath << " is not a file \n";
        return -1;
    }

    // Check if the destinantion alreadt exists!
    if(find_dir_entry(destpath, root_dir)) {
        std::cerr << "Error Destinantion file already exists \n";
        return -1;  
    }

    uint16_t blk = src_entry->first_blk;
    uint16_t prev_blk = FAT_FREE;   //As we copy each block from the source to the destination, prev_blk will be updated to link the blocks together in the FAT.
    uint16_t first_blk_dest = FAT_FREE;
    uint32_t size = src_entry->size;
    uint8_t buffer[BLOCK_SIZE];
    while(blk != FAT_EOF) {
        //read the source Block
        if(disk.read(blk, buffer) != 0) {
            std::cerr << "Error: Failed to read block " << blk << " from disk \n";
            return -1;
        }

        //Find a free block for the destinantion file
        uint16_t dest_blk = FAT_FREE;
        for(uint16_t i = 0; i < BLOCK_SIZE / 2; i++) {
            if(fat[i] == FAT_FREE) {
                dest_blk = i;
                break;
            }
        }

        if(dest_blk == FAT_FREE) {
            std::cerr << "Error: No free blocks available on disk \n";
            return -1;
        }      


        //Write the block to the destinantion
        if(disk.write(dest_blk, buffer) != 0) {
            std::cerr << "Error: Failed to write to block " << dest_blk << " \n";
            return -1;
        }

        //update the FAT
        if (prev_blk != FAT_FREE) {
            fat[prev_blk] = dest_blk;
        }
        else {
            first_blk_dest = dest_blk;
        }
        prev_blk = dest_blk;
        //morve to the next block in the source file
        blk = fat[blk];
    }

    //mark the end of the dest file in the FAT
    if (prev_blk != FAT_FREE){
        fat[prev_blk] = FAT_EOF;
    }

    //create a new directory entry for the destinantion file
    for (int i = 0; i < BLOCK_SIZE / sizeof(dir_entry); ++i){
        if (root_dir[i].file_name[0] == '\0'){
            std::strncpy(root_dir[i].file_name, destpath.c_str(), sizeof(root_dir[i].file_name) - 1);
            root_dir[i].file_name[sizeof(root_dir[i].file_name) - 1] = '\0';
            root_dir[i].size = src_entry->size;
            root_dir[i].first_blk = first_blk_dest;
            root_dir[i].type = TYPE_FILE;
            root_dir[i].access_rights = src_entry->access_rights;
            break;
        }

    }

    // Write the updated FAT and root directory back to disk
    if (disk.write(FAT_BLOCK, reinterpret_cast<uint8_t*>(fat)) != 0) {
        std::cerr << "Error: Failed to write FAT to disk\n";
        return -1;
    }

    if (disk.write(ROOT_BLOCK, reinterpret_cast<uint8_t*>(root_dir)) != 0) {
        std::cerr << "Error: Failed to write root directory block to disk\n";
        return -1;
    }

    return 0;
}

// mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
// or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int
FS::mv(std::string sourcepath, std::string destpath)
{
    std::cout << "FS::mv(" << sourcepath << "," << destpath << ")\n";
    return 0;
}

// rm <filepath> removes / deletes the file <filepath>
int
FS::rm(std::string filepath)
{
    std::cout << "FS::rm(" << filepath << ")\n";
    return 0;
}

// append <filepath1> <filepath2> appends the contents of file <filepath1> to
// the end of file <filepath2>. The file <filepath1> is unchanged.
int
FS::append(std::string filepath1, std::string filepath2)
{
    std::cout << "FS::append(" << filepath1 << "," << filepath2 << ")\n";
    return 0;
}

// mkdir <dirpath> creates a new sub-directory with the name <dirpath>
// in the current directory
int
FS::mkdir(std::string dirpath)
{
    std::cout << "FS::mkdir(" << dirpath << ")\n";
    return 0;
}

// cd <dirpath> changes the current (working) directory to the directory named <dirpath>
int
FS::cd(std::string dirpath)
{
    std::cout << "FS::cd(" << dirpath << ")\n";
    return 0;
}

// pwd prints the full path, i.e., from the root directory, to the current
// directory, including the currect directory name
int
FS::pwd()
{
    std::cout << "FS::pwd()\n";
    return 0;
}

// chmod <accessrights> <filepath> changes the access rights for the
// file <filepath> to <accessrights>.
int
FS::chmod(std::string accessrights, std::string filepath)
{
    std::cout << "FS::chmod(" << accessrights << "," << filepath << ")\n";
    return 0;
}
/*##################################### HELP FUNCTIONS ###############################*/

/*Function to get different parts of the filenam*/
void
FS:: file_parts(std::string filepath, std::string *filename, std::string *directury_path){
    
    // if filepath is /home/user/docs/file.txt:

    // find_last_of("/\\") finds the 16 carecters.
    // *filename becames file.txt.
    // *directory_path becomes /home/user/docs./

    size_t last_separater = filepath.find_last_of("/\\");

    if (last_separater != std::string::npos){
        *filename = filepath.substr(last_separater + 1); 
        *directury_path = filepath.substr(0 , last_separater); 
    }
    else {

        *filename = ""; 
        *directury_path = filepath; 
    }

}


dir_entry* 
FS::find_directury_entry(std::string filename){

    // Iterate thought all directory enteries in the current working directory
    for ( int i = 0; i < DIR_SIZE; i ++){
        // Compare each filename in the directory entries with the given filename
        if (strncmp(cwd.enteries[i].file_name , filename.c_str(),56) == 0){
        // If the filename matches, return a pointer to the matching directory entry
            return &cwd.enteries[i];
        }

    // Return nullptr if no matching directory entry was found
        return nullptr;
    }

}

int
FS::fin_a_empty_block(){
    int block_number = -1 ;
    for (int i = 0; i < FAT_ENTRIES ; i++){
        if (fat[i] == FAT_FREE){
            block_number = i; 
            break;
        }
    }
    if (block_number == -1){
        std::cerr << "FS::find_a_empty_block: No free blocks" << std::endl;
        return -1;
    }
    return block_number; 

}


////// Help function for cat and ls and cp
//Heleper function to find the directory entry for a given file 
dir_entry* FS::find_dir_entry(const std::string& filename, dir_entry* root_dir){
    for (int i = 0; i < BLOCK_SIZE / sizeof(dir_entry); ++i){
        if(std::strcmp(root_dir[i].file_name, filename.c_str()) == 0 ) {
            return &root_dir[i];
        }
    }
    return nullptr;
}
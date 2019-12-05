#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 /*------------------------------------------------------------------*/
/*Given data/ sizes                                                 */
/*------------------------------------------------------------------*/
#define SB_MAGIC_NUMBER 0xACBD0005
#define BLOCK_SIZE 1024  //1024 byte sectors
#define MAX_FILE_NAME_LENGTH 16
#define MAX_EXTENSION_LENGTH 3
#define MAXFILENAME 20

//assume
#define NUM_DISK_BLOCKS 120
#define NUM_INODES 40

typedef struct superblock { 
    uint64_t magicNumber; 
	uint64_t blockSize;
	uint64_t fileSysSize;
	uint64_t iNodeTableLength;
	uint64_t rootDirectory; 
} superblock;

typedef struct INode { 
    unsigned int mode;
	unsigned int linkCount;
	unsigned int uid;
	unsigned int gid;
	unsigned int isize; 
	unsigned int diskBlockPointer[25];
	unsigned int indirectPointer;
	int ifree;
} INode;

typedef struct InderectBlockP{
	int block[BLOCK_SIZE / sizeof(int)];
} InderectBlockP;

typedef struct fileDescriptor {
	uint64_t free; //1 if free, 0 otherwise
	uint64_t iNodeNum;
	uint64_t readp;
	uint64_t writep;
} fileDescriptor;

typedef struct	directoryEntry { //an entry in the director that maps file name to i-Node
	char fileName [MAXFILENAME];
	uint64_t correspondingINode;
	uint64_t active; //0 not active, 1 is
} directoryEntry;

typedef struct directory{
	directoryEntry list[NUM_INODES];
	int numEntries;
} directory;

//helper methods
void init_superblock();
void init_directory();
	int find_dir_freespot();
void init_fdt();
	int exists_in_fdt(char* name);
	int find_fdt_freespot();
void init_inodetable();
	int find_inode_from_name(char *name);
	int find_inode_freespot();
void init_bmap();
	int get_first_empty_block();

//main methods

void mksfs(int fresh);

int sfs_getnextfilename(char *fname); // get the name of the next file in directory

int sfs_getfilesize(const char* path); // get the size of the given file

int sfs_fopen(char *name); // opens the given file

int sfs_fclose(int fileID); // closes the given file

int sfs_frseek(int fileID, int loc); // seek (Read) to the location from beginning

int sfs_fwseek(int fileID, int loc); // seek (Write) to the location from beginning

int sfs_fwrite(int fileID, char *buf, int length); // write buf characters into disk

int sfs_fread(int fileID, char *buf, int length); // read characters from disk into buf

int sfs_remove(char *file); // removes a file from the filesystem
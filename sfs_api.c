#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "sfs_api.h"
#include "disk_emu.h"
 
/*------------------------------------------------------------------*/
/*                         Alisa Gagina         
							260770497   
							Dec 3 2019
						Simple File System                 
 */
/*------------------------------------------------------------------*/
char sfsname[] = "ALISAFS";
int error = 0; //0 means no error

/*------------------------------------------------------------------*/
/*Take care of the memory space first  and helper methods           */
/*------------------------------------------------------------------*/
superblock sb;

void init_superblock() { //0th block
    sb.magicNumber = SB_MAGIC_NUMBER;
    sb.blockSize = BLOCK_SIZE;    
    sb.fileSysSize = NUM_DISK_BLOCKS;
    sb.iNodeTableLength = NUM_INODES;
    sb.rootDirectory = 0; //data block at 0

    write_blocks(0,  1 , &sb);
}

directory dir;
directory tempd; //temp directory for sfs_remove
int currentDirectoryIndex; //needed for sfs_getnextfilename

void init_directory(){	//2nd block
	dir.numEntries = 0;
	for(int counter = 0; counter<NUM_INODES; counter++){ //initialize all slots to empty
		dir.list[counter].active = 0;
		dir.list[counter].correspondingINode=-1;
		strcpy(dir.list[counter].fileName, "");
	}
	//save on disk
	  write_blocks(2, 1, &dir);

	  //update root
	 void * buffer = malloc(1024*sizeof(char));
  	 read_blocks(1, 1, buffer);
  	 INode* inode = (INode*) buffer;
  	 free(buffer);

     inode[0].isize = 1;
     inode[0].ifree = 0;
     inode[0].diskBlockPointer[0] = 2;
 	//update root on disk
  	write_blocks(1, 1, inode); 


	currentDirectoryIndex = 0; 
}


	int find_dir_freespot(){
		for(int counter = 0; counter<NUM_INODES; counter++){
			if (dir.list[counter].active == 0){
				return counter; //found a free spot dir, return index
			}
		}  
		return -1; //no space in dir
	}


fileDescriptor fdt[NUM_INODES];//inode num = index
void init_fdt(){
	for(int counter = 0; counter<NUM_INODES; counter++){
		fdt[counter].readp = 0;
		fdt[counter].writep = 0;
		fdt[counter].iNodeNum = -1; //inode must be greater than 0 so -1  means inode fd entry is free.
		fdt[counter].free = 1; //yes, free
	}
}
	int exists_in_fdt(char* name){ //aka is file open?
		int id = find_inode_from_name(name);
		for(int counter = 0; counter<NUM_INODES; counter++){
			if (fdt[counter].iNodeNum == id){
				return counter; //found in fdt, return index
			}
		}  
		return -1; //not in fdt
	}

	int find_fdt_freespot(){
		for(int counter = 0; counter<NUM_INODES; counter++){
			if (fdt[counter].free == 1){
				return counter; //found in fdt, return index
			}
		}  
		return -1; //no space in fdt
	}

INode inodeTable[NUM_INODES];

void init_inodetable(){  //1st block
	for(int counter = 0; counter<NUM_INODES; counter++){
		inodeTable[counter].isize=0;
		inodeTable[counter].ifree=1; //yes inode is free
		for (int bp =0; bp < 25; bp++){
			inodeTable[counter].diskBlockPointer[bp]=-1;
		}
		inodeTable[counter].indirectPointer = -1;
	}
	 write_blocks(1, 1, &inodeTable);
}
	int find_inode_from_name(char *name){
		int location = -1;
		for (int i =0; i < NUM_INODES; i++){
			if(strcmp(dir.list[i].fileName, name)==0 && dir.list[i].active ==1){ //if created and name matches
				location = dir.list[i].correspondingINode;
			}
		}
		return location;
	}
	int find_inode_freespot(){
		for(int counter = 1; counter<NUM_INODES; counter++){
			if (inodeTable[counter].ifree == 1){
				return counter; //found a free spot inodeTable, return index
			}
		}  
		return -1; //no space in inodeTable
	}


int bitmap[NUM_DISK_BLOCKS];

void init_bmap(){ //3rd block
	//non-empty
	bitmap[0]=1; //superblock
	bitmap[1]=1; //inodetable
	bitmap[2]=1; //directory
	bitmap[3]=1; //bitmap

	//empty
	for(int i=4; i<NUM_DISK_BLOCKS; i++){
   		bitmap[i] = 0;
  	}
  	write_blocks(3, 1, &bitmap);
}
	//Iterate through bit map on disk and find first empty block i.e 0 value
	int get_first_empty_block(){
	  //Itialize a buffer so we read from disk
	 void * buffer = malloc(1024*(sizeof(int)));
	 read_blocks(3, 1, buffer);
	 int * diskBitmap = (int*) buffer;
	  free(buffer);

	  for(int i=0; i<NUM_DISK_BLOCKS; i++){
	    if(diskBitmap[i] != 1){
	      return i;
	    }
	  }
	  return -1;
	}

 /*------------------------------------------------------------------*/
/*                          MSKFS                                   */
/*Creates / reopens the file system                                 */
/*There are 5 helper functions used (defined above)                 */
/*------------------------------------------------------------------*/
void mksfs(int fresh){
		
	if(fresh){    //make new file system
		remove(sfsname);

		error= init_fresh_disk(sfsname, BLOCK_SIZE, NUM_DISK_BLOCKS);//initialize a new file system filled with 0’s (disk_emu.c)
		if(error==-1){
			//printf("!!!         error in mksfs           !!!");
		}

		init_superblock(); //make superblock and specify its values
		//initialize and write the rest of structures to disk
		init_inodetable();

		init_fdt();
		init_directory();	
		init_bmap();

	}else{ //file system already exists
	
		//load the file system from disk
		error= init_disk(sfsname, BLOCK_SIZE, NUM_DISK_BLOCKS);
		if(error==-1){
			//printf("!!!         error in mksfs           !!!");
		}
		read_blocks(1, 1, &inodeTable);
		read_blocks(2, 1, &dir);
		read_blocks(3, 1, &bitmap);
		//Reset inode table, freebitmap, directory cache (can this be done using init again?)
	}
}

 /*------------------------------------------------------------------*/
/*                   SFS_GETNEXTFILENAME                             */
/* Find the next file being pointed in directory                */
/*write filename into fname                                         */                     
/*Return 0 on failure, 1 on success                                */
/*------------------------------------------------------------------*/
int sfs_getnextfilename(char *fname){
	int counted=0;
	  for(int i = 1; i < NUM_INODES ; i++){ //should I skip first inode since it is the directory
            if(inodeTable[i].ifree == 1){
                counted++;
            }
        }

	if(currentDirectoryIndex == counted){
		currentDirectoryIndex=0;
		return 0;
	}else{
		strcpy(fname, dir.list[currentDirectoryIndex].fileName); 
		currentDirectoryIndex++;
		return 1;
	}
}

 /*------------------------------------------------------------------*/
/*                   SFS_GETFILESIZE                                */
/* find file being pointed to                                       */      
/* cycle through directory to find size of inode                     */              
/*Return size on success, -1 on failure                                */
/*------------------------------------------------------------------*/
int sfs_getfilesize(const char* path){
	int size = -1;
	for (int i =1; i < NUM_INODES; i++){
		if(strcmp(dir.list[i].fileName, path)==0 && dir.list[i].active ==1){ //dir name matches & active
			int inodeNum = dir.list[i].correspondingINode;
			size = inodeTable[inodeNum].isize;
		}
	}
	return size;
}

 /*------------------------------------------------------------------*/
/*                   SFS_FOPEN                                     */
/* see if file already exists in directory                          */     
/* if yes, open it / check if already opened                          */ 
/* if not, make one, check for name length  / existance / extension    */ 
/* update fdt, inode, directory                    		              */              
/*Return fdt location on success, -1 on failure                       */
/*------------------------------------------------------------------*/
int sfs_fopen(char *name){
	//check for file existance
	int location = find_inode_from_name(name);
	//we found the file
	if(location>0){
		//it is open (already in fdt)
		if (exists_in_fdt(name) != -1){
			//printf("!!!!       Already open      !!!!\n");
			return -1; 
		}
		//it is closed; so we add to fdt

		int freespot = find_fdt_freespot(); //-1 if no space

		if (freespot == -1){
			//printf("No space in fdt");
			return -1; 
		}else{
			fdt[freespot].iNodeNum= location;
			fdt[freespot].free = 0;
			fdt[freespot].readp = 0;
			int endoffile = inodeTable[location].isize;
			fdt[freespot].writep = endoffile; //find end of file
			return freespot;
		}
		

	}else{ //file doesn't exist, need to make a new one
		
		//first we find a free inode
		int inodeid = find_inode_freespot(); //-1 if no space
		if (inodeid == -1){
			//printf("No space in inodeTable");
			return -1;
		}
		//also check for first free directory spot
		int dirid = find_dir_freespot(); //-1 if no space
		if (dirid == -1){
			//printf("No space in directory");
			return -1;
		}
		//update inode 
		inodeTable[inodeid].ifree = 0;
		for (int j= 0; j<25; j++){
			inodeTable[inodeid].diskBlockPointer[j]= -1;
		}
		//update dir
		dir.list[dirid].correspondingINode = inodeid;
	 	dir.list[dirid].active = 1;

	 	//check name for correctness (max 16)
	  		int i = 0;
			while (name[i] != '.'){
				if (i >= MAX_FILE_NAME_LENGTH){				
					//printf("File name is too long");
					return -1;
				}
				i++;
			}

		// We check extension for correctness (max 3)
			int j = 0;
			while (name[i + 1 + j] != '\0'){
				if (j >= MAX_EXTENSION_LENGTH){
					//printf("File extension is too long");
					return -1;
				}
				j++;
			}
		strcpy(dir.list[dirid].fileName, name); //filename is in a buffer of 20 chars (16 + '.' + 3)
		dir.numEntries++;

		//update disk
		write_blocks(1, 1, &inodeTable);
		write_blocks(2, 1, &dir);

		//find first empty fdt spot
		int freespot = find_fdt_freespot(); //-1 if no space

		if (freespot == -1){
			//printf("No space in fdt");
			return -1;
		}

		//update fdt, file is open
		fdt[freespot].iNodeNum= inodeid;
		fdt[freespot].free = 0;
		fdt[freespot].readp = 0;
		fdt[freespot].writep = 0; //nothing written yet

		return freespot;
	}

	return -1;
}

 /*------------------------------------------------------------------*/
/*                              SFS_FCLOSE                        */
/* see if file is open if fdt                                  */     
/* close if it is                                                */ 
/* if already closed, notify                                     */
/* Return 0 on success, -1 on failure                                */
/*------------------------------------------------------------------*/
int sfs_fclose(int fileID){
	//first lets error handle
	if (fileID < 0 || fileID > NUM_INODES){ //no such file
		return -1;
	}else if(fdt[fileID].free == 1){  //no file at location
		//printf("\n a;ready closed");
		return -1;
	}else{ //update fdt
		fdt[fileID].iNodeNum= -1;
		fdt[fileID].free = 1;
		fdt[fileID].readp = 0;
		fdt[fileID].writep = 0; 
		return 0;
	}
}

/*------------------------------------------------------------------*/
/*                   SFS_FRSEEK  & SFS_FWSEEK                       */
/* move rpointer / wpointer of file from start to loc                */     
/* Return 0 on success, -1 on failure                                */
/*------------------------------------------------------------------*/
int sfs_frseek(int fileID, int loc){
	if(fdt[fileID].free==1){
		//printf("file closed");
		return -1; 
	}
	int inodeid= fdt[fileID].iNodeNum;
	int size = inodeTable[inodeid].isize;
	if (loc < 0 || loc > size){
		return -1;
	}else{
		fdt[fileID].readp = loc;
		return 0;
	}
}

int sfs_fwseek(int fileID, int loc){
	if(fdt[fileID].free==1){
		//printf("file closed");
		return -1; 
	}
	int inodeid= fdt[fileID].iNodeNum;
	int size = inodeTable[inodeid].isize;
	if (loc < 0 || loc > size){
		return -1;
	}else{
		fdt[fileID].writep = loc;
		return 0;
	}
}

/*------------------------------------------------------------------*/
/*                             SFR_WRITE                             */
/* find file & corresponding inode                                   */     
/* figure out which block we're writing to (if we need ind block)    */   
/* start cycling through blocks one by one                           */   
/* save file to disk                                                 */   
/* Return number bytes written on success, -1 on failure           */
/*------------------------------------------------------------------*/
int sfs_fwrite(int fileID, char *buf, int length){
	
	//first check if file is open
	if(fdt[fileID].free==1){
		//printf("File isn't open\n");
		return -1; 
	}

	//find file /inode
	int inodeid = fdt[fileID].iNodeNum;
 	INode i = inodeTable[inodeid];
 	int bytesWritten=0;
 	int bytesToWrite=length;
 	void* data = malloc(BLOCK_SIZE);         //temp buffer to write into
	memset(data, '\0', BLOCK_SIZE);

	InderectBlockP indb = {};            //initialize indirect block
	if(bytesToWrite >0 && i.indirectPointer <= 0){
		i.indirectPointer = get_first_empty_block();
		if (i.indirectPointer < 0){
			return -1; //no space
		}
		bitmap[i.indirectPointer]= 1;
	}else{
		read_blocks(i.indirectPointer, 1, &indb);
	}

 	while(bytesToWrite > 0){
 		memset(data, '\0', BLOCK_SIZE);                              //clear buffer
 		int currentBlockNum = fdt[fileID].writep / 1024;               //block
 		//printf("writep is %ld \n", fdt[fileID].writep);
 		//printf("vurrent block num is %d \n", currentBlockNum);
 		int byteInBlock = fdt[fileID].writep %1024;                  //position in said block
 		int toWrite; //amount we are writing in this specific block
 		if (bytesToWrite > (BLOCK_SIZE - byteInBlock)){
				toWrite = BLOCK_SIZE - byteInBlock;                   //something is already written in the block
		} else{
				toWrite = bytesToWrite;
		}
		int blockIsNew = 0;
		int diskIndex;

		if(currentBlockNum >= 25){ //indirect
			diskIndex = indb.block[currentBlockNum-25];
			if(diskIndex <= 0){ //if we need to make a new one
				blockIsNew = 1;
				diskIndex = get_first_empty_block();
				if (diskIndex == -1){
					break; // no space
				}
				bitmap[diskIndex]= 1;
				indb.block[currentBlockNum-25] = diskIndex;
			}

		}else{    //no need for indirect block, just write to inode directly
			diskIndex = i.diskBlockPointer[currentBlockNum];
			//printf("diskInd is %d, inode num is %d \n", diskIndex, inodeid);
			if (diskIndex < 0){
					blockIsNew = 1; 
					// Get a new bitmap index
					diskIndex = get_first_empty_block();
					if (diskIndex == -1){
							// Disk is full!
							break;
					}
					// Mark it unfree
					bitmap[diskIndex]=1;
					i.diskBlockPointer[currentBlockNum] = diskIndex;
				}
		}
		// If the block is already partially full, then we need to load the old data
		if (!blockIsNew){
  				  read_blocks(diskIndex, 1, data);
		}

		// Copy the desired amount of data onto it
		memcpy(data + byteInBlock, buf, (size_t) toWrite);
		 if (diskIndex + 1 > NUM_DISK_BLOCKS){
        	//printf("write blocks prevention, currentBlockNum is %d \n", currentBlockNum);
        	break;
    	}else{
			write_blocks(diskIndex, 1, data);
		}
		// Save how much we have written to the file pointer
		fdt[fileID].writep += toWrite;

		// Track how much we have written. 
		bytesWritten += toWrite;
		bytesToWrite -= toWrite;
		buf += toWrite;

		// update fdt file size change
		if (fdt[fileID].writep > i.isize){
				i.isize = fdt[fileID].writep;
		}
	}
	//wrote all the data we could
	// Write the data block to disk
	
	if (i.indirectPointer > -1){
			write_blocks(i.indirectPointer, 1, &indb);
	}
	
	inodeTable[inodeid]=i;
	// Free the new buffer
	free(data);

	// Save the data
	 write_blocks(1, 1, &inodeTable);
 	 write_blocks(3, 1, &bitmap);

	return bytesWritten;
}
/*------------------------------------------------------------------*/
/*                             SFR_Read                             */
/* find file & corresponding inode                                   */     
/* figure out which block we're reading from (if we need ind block)    */   
/* start cycling through blocks one by one                           */   
/* Return number bytes read on success, -1 on failure                */
/*------------------------------------------------------------------*/
int sfs_fread(int fileID, char *buf, int length){
//Check if file is open
  if(fdt[fileID].free){
    return -1;
  }
  //printf("file rp is %d \n", fdt[fileID].readp);
    //printf("file wp is %d \n", fdt[fileID].writep);
  //get inode
	int inodeid = fdt[fileID].iNodeNum; 
 	INode i = inodeTable[inodeid];
 	if((i.isize - fdt[fileID].writep)<0){    //debugging, not really necessary
 		fdt[fileID].readp= (fdt[fileID-1].readp)+length;
 	}
 	//if there's nothing to read, just end early
  if(length==0){
     return 0;
  }

  int totalRead = 0;
  int toRead = length;

  if(i.isize < length){
  	toRead = i.isize;    //if the file we're reading from is smaller, so that we don't read past \0
  }

  //indirect block?
  InderectBlockP indb = {};
  if (i.indirectPointer > -1){
  	read_blocks(i.indirectPointer, 1, &indb);
  }
  //temp buffer to read to
  void* data = malloc(BLOCK_SIZE);
  memset(data, '\0', BLOCK_SIZE);

  while(toRead > 0){  //cycle through blocks one by one
  	int currentBlockNum = fdt[fileID].readp / BLOCK_SIZE;
  	int byteInBlock = fdt[fileID].readp % BLOCK_SIZE;
  	int currentToRead;
  	if (toRead > (BLOCK_SIZE - byteInBlock)){
  		currentToRead = BLOCK_SIZE - byteInBlock;  
  	}else{
  		currentToRead = toRead;
  	}
  	memset(data, '\0', BLOCK_SIZE);

  	if (currentBlockNum >= 25){
       	read_blocks(indb.block[currentBlockNum-25], 1, data); //if reading from indirect block
  	 }else{
  		read_blocks(i.diskBlockPointer[currentBlockNum], 1, data); //if reading from inode table
  	}
  	 // Copy the desired amount of memory into the output buffer
	memcpy(buf, data + byteInBlock, (size_t) currentToRead);
	// Advance the r pointer
	fdt[fileID].readp += currentToRead;
	//update counters
	totalRead += currentToRead;
	toRead -= currentToRead;
	buf += currentToRead;
  }
  free(data);
  return totalRead;
}
/*------------------------------------------------------------------*/
/*                             SFR_REMOVE                           */
/* find file, directory entry, inode                                */     
/* reset directory & reshuffle to get rid of the gap               */   
/* reset bitmap, inode table, fdt and save to disk                   */   
/* Return 0 on success, -1 on failure                              */
/*------------------------------------------------------------------*/
int sfs_remove(char *file){

	//find file in fdt
	int fdtindex = exists_in_fdt(file);
	if (fdtindex != -1){
		//printf("\n file is open, can't remove");
		return -1;
	}
	//find file in directory 
	int dirindex = -1;
	for(int counter = 0; counter<NUM_INODES; counter++){
			if (dir.list[counter].active == 1 && strcmp(dir.list[counter].fileName, file)){
				dirindex = counter; 
			}
		
	}	  
	if (dirindex == -1){
		return -1;
	}

	//find file in inodeTable
	int iindex = find_inode_from_name(file);
	

	//update directory
	dir.list[dirindex].correspondingINode = -1;
	strcpy(dir.list[dirindex].fileName, "");
	dir.list[dirindex].active = 0;
	if (dirindex <= currentDirectoryIndex && currentDirectoryIndex != 0){
		currentDirectoryIndex --;
	}
	//get rid of the empty spot in directory

			tempd.numEntries = 0;
			for(int counter = 0; counter<NUM_INODES; counter++){ //initialize all slots to empty
				tempd.list[counter].active = 0;
				tempd.list[counter].correspondingINode=-1;
				strcpy(tempd.list[counter].fileName, "");
			}

			int tempindex = 0;
			for (int j = 0; j < NUM_INODES; j++){
				if(dir.list[j].active==1){
					tempd.list[tempindex].active =1;
					strcpy(tempd.list[tempindex].fileName, dir.list[j].fileName);
					tempd.list[tempindex].correspondingINode = dir.list[j].correspondingINode;
					tempindex++;
				}
			}

			for(int k = 0; k< NUM_INODES; k++){
				dir.list[k].active = 0;
				dir.list[k].correspondingINode=-1;
				strcpy(dir.list[k].fileName, "");
			}

			for(int counter = 0; counter < NUM_INODES; counter++){
				if(tempd.list[counter].active){
					dir.list[counter].active = 1;
					dir.list[counter].correspondingINode=tempd.list[counter].correspondingINode;
					strcpy(dir.list[counter].fileName, tempd.list[counter].fileName);
				}
			}

			write_blocks(2, 1, &dir);
	
	//update bitmap
	for (int j= 0; j<25; j++){
		if(inodeTable[iindex].diskBlockPointer[j]!= -1){
			bitmap[inodeTable[iindex].diskBlockPointer[j]]=0;
		}
	}

	//update inode table
	inodeTable[iindex].isize = 0;
	inodeTable[iindex].ifree = 1;
	for (int j= 0; j<25; j++){
		inodeTable[iindex].diskBlockPointer[j]= -1;
	}
	inodeTable[iindex].indirectPointer = -1;

	//fdt table
	fdt[fdtindex].iNodeNum = -1;
	fdt[fdtindex].readp=0;
	fdt[fdtindex].writep=0;
	fdt[fdtindex].free=1;

	//update disk
	 write_blocks(1, 1, &inodeTable);
 	 write_blocks(3, 1, &bitmap);
  	write_blocks(2, 1, &dir);

  	return 0;
}
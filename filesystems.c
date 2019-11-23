#include <stdint.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define BLOCK_SIZE 8192
#define NUM_BLOCKS 4226
#define NUM_FILES 128

// File attributes
#define NOTHING 0
#define HIDDEN 1
#define READONLY 2
#define BOTH 3


uint8_t blocks[NUM_BLOCKS][BLOCK_SIZE];

struct Directory_Entry {
  int8_t valid;
  char name[255];
  uint32_t inode;
};

struct Inode {
  uint8_t attributes;
  uint32_t size;
  uint32_t blocks[1250];
};


struct Directory_Entry * dir;
struct Inode *inodeList;
uint8_t *freeBlockList;
uint8_t *freeInodeList;


void intializeBlockList() {
  int i;
  for(i=0;i<NUM_BLOCKS;i++) {
    freeBlockList[i]=1;
  }
}

void intializeInodeList() {
  int i;
  for(i=0;i<NUM_FILES;i++) {
    freeInodeList[i]=1;
  }
}

void intializeDirectory() {
  int i;
  for (i=0;i<NUM_FILES;i++) {
    dir[i].valid=0;
    memset(dir[i].name,0,255);
    dir[i].inode=-1;
  }
}

void intializeInodes() {
  int i;
  for (i=0;i<NUM_FILES;i++) {
    int j;
    inodeList[i].attributes=0;
    inodeList[i].size=0;
    for (j=0;j<1250;j++) {
      inodeList[i].blocks[j]=-1;
    }
  }
}

int findFreeInode() {
  int ret =-1;
  int i;

  for(i=0;i<NUM_FILES;i++) {
    if(freeInodeList[i]==1) {
      ret=1;
      freeInodeList[i]=0;
      break;
    }
  }
  return ret;
}

int findFreeDirectory() {
  int ret =-1;
  int i;

  for(i=0;i<NUM_FILES;i++) {
    if(dir[i].valid==-1) {
      ret=1;
      dir[i].valid=0;
      break;
    }
  }
  return ret;
}

int findFreeBlock() {
  int ret =-1;
  int i;

  for(i=0;i<NUM_BLOCKS;i++) {
    if(freeBlockList[i]==1) {
      ret=1;
      freeBlockList[i]=0;
      break;
    }
  }
  return ret;
}

int filenameSize(char *filename) {
  int res = 0;
  char curr_char = filename[0];
  while(curr_char != '\0') {
    curr_char = filename[res];
    ++res;
  }
  return --res;
}

int getFileIndex(char* filename) {
  for(int i = 0; i < NUM_FILES; i++) {
    if(!strcmp(dir[i].name, filename)) {
      return i;
    }
  }
  return -1;
}

// initializes a new file system
void createfs(char *filename) {
  int i;
  memset(blocks, 0,NUM_BLOCKS*BLOCK_SIZE);
  FILE *fd=fopen(filename,"w");
  intializeDirectory();
  intializeInodeList();
  intializeBlockList();
  intializeInodes();
  fwrite(blocks,BLOCK_SIZE,NUM_BLOCKS,fd);
  fclose(fd);
}

// opens a file image and writes its contents into blocks
void open(char * filename) {
  FILE *fd = fopen(filename,"r");
  fread(blocks,BLOCK_SIZE,NUM_BLOCKS,fd);
  fclose(fd);
}

// write the block data into a file
void close(char * filename) {
  FILE *fd = fopen(filename,"w");
  fwrite(blocks,BLOCK_SIZE,NUM_BLOCKS,fd);
  fclose(fd);
  memset(blocks, 0,NUM_BLOCKS*BLOCK_SIZE);
}

// gets size that is remaining in the disk image
long df() {
  long free_size = 0;
  for(int i = 0; i < NUM_FILES; i++) {
    if(dir[i].valid == 0) {
      // if the file is not occupied then it adds the
      // file size to the free space
      free_size += BLOCK_SIZE;
    }
  }
  return free_size;
}

// takes the name of file for input, if it exists
// then it opens the file and checks it for the
// certain criteria and copies the content into the 
// diskimage
void put(char * filename) {
  struct stat buffer;
  if(stat(filename, &buffer) != 0) {
    printf("put error: the File with the name %s does not exist\n",filename);
  } else {
    FILE *fd = fopen(filename,"r");
    // gets the file size in bytes
    long file_size = buffer.st_size;
    if(file_size >= 10240000 || file_size > df()) {
      // checks if the file size is greater or equal to than 10MB
      // and also if there is enough free space in the disk image
      printf("put error: the File with the name %s is too big\n",filename);
    } if(filenameSize(filename) > 255) {
      // checks if the filename is greater than 255 characters
      printf("put error: the Filename is too long\n");
    }
  }

  dir[0].valid = 1;
  strcpy(dir[0].name,"hi.there");
  dir[0].inode = 0;
  inodeList[dir[0].inode].attributes = NOTHING;
  inodeList[dir[0].inode].size = 100;
  
  dir[1].valid = 1;
  strcpy(dir[1].name,"hey.there");
  dir[1].inode = 1;
  inodeList[dir[1].inode].attributes = NOTHING;
  inodeList[dir[1].inode].size = 500;

  dir[2].valid = 1;
  strcpy(dir[2].name,"hello.there");
  dir[2].inode = 2;
  inodeList[dir[2].inode].attributes = NOTHING;
  inodeList[dir[2].inode].size = 300;
  
  dir[3].valid = 1;
  strcpy(dir[3].name,"howdy");
  dir[3].inode = 3;
  inodeList[dir[3].inode].attributes = NOTHING;
  inodeList[dir[3].inode].size = 800;
}

// lists all the files in the file system
void list() {
  int i;
  for(i = 0; i<NUM_FILES; i++){
    if(dir[i].valid == 1 && (inodeList[dir[i].inode].attributes != HIDDEN && inodeList[dir[i].inode].attributes != BOTH)){
      // checks to see if the file in the file system
      // is either hidden or both hidden and read-only
      printf("%6d\t%s\n",inodeList[dir[i].inode].size,dir[i].name);
    }
  }
}

// changes the attributes for the file
void attrib(char* attrib, char* filename) {
  char functionality = attrib[0];
  char attribute_to_change = attrib[1];
  int index = dir[getFileIndex(filename)].inode;
  if(index == -1) {
    printf("attrib error: File not found\n"); 
    return;
  }
  if(functionality == '+') {
    // if the attribute is supposed to be added
    // then add to the attribute value depending
    // on user input
    if(attribute_to_change == 'h' && (inodeList[index].attributes == NOTHING || inodeList[index].attributes == READONLY)) {
      inodeList[index].attributes += HIDDEN;
    } else if(attribute_to_change == 'r' && (inodeList[index].attributes == NOTHING || inodeList[index].attributes == HIDDEN)){
      inodeList[index].attributes += READONLY;
    }
  } else if(functionality == '-') {
    // if the attribute is supposed to be removed
    // decrement the attribute value if if has that
    // attribute
    if(attribute_to_change == 'h' && (inodeList[index].attributes == HIDDEN || inodeList[index].attributes == BOTH)) {
      inodeList[index].attributes -= HIDDEN;
    } else if(attribute_to_change == 'r' && (inodeList[index].attributes == READONLY || inodeList[index].attributes == BOTH)){
      inodeList[index].attributes -= READONLY;
    }
  } else {
    printf("attrib error: Invalid attrib\n");
    return;
  }
}

// deletes a file from the filesystem
void del(char* filename) {
  int index = getFileIndex(filename);
  if(index == -1) {
    // checks if the file exists in the file system
    // if not it returns an error
    printf("del error: File not found\n");
  } else {
    if(inodeList[dir[index].inode].attributes == READONLY || inodeList[dir[index].inode].attributes == BOTH) {
        // if the file is fround and is read-only
        // then it returns an error
        printf("del error: File is marked read-only\n");
    } else {
      // else the file's valid is set to 0, essentially
      // unoccupying it
      dir[index].valid = 0;
    }
  }
}

int main() {
  dir=(struct Directory_Entry*)&blocks[0];
  inodeList =(struct Inode*)&blocks[6];
  freeInodeList=(uint8_t*)&blocks[4];
  freeBlockList=(uint8_t*)&blocks[5];

  // create fs 
  createfs("disk.image");
  
  // opens fs
  open("disk.image");
  
  // free space in fs
  printf("current free space is %ld\n", df());
  
  // put into fs
  put("test.txt");
  
  // change file attributes
  attrib("+h","hi.there");
  attrib("+r","hi.there");

  // list files in fs
  list();
  
  // free space in fs again
  printf("current free space is %ld\n", df());

  // change file attributes
  attrib("+h","hi.there");

  // deletes file from the filesystem
  del("hi.there");
  
  // list files in fs
  list();
  
  // free space in fs again
  printf("current free space is %ld\n", df());

  // close fs
  close("disk.image");
  
  return 0;
}

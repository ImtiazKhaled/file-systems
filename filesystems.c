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
#define HIDDENANDREADONLY 2


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
  for(i=0;i<4226;i++) {
    freeBlockList[i]=1;
  }
}

void intializeInodeList() {
  int i;
  for(i=0;i<128;i++) {
    freeInodeList[i]=1;
  }
}

void intializeDirectory() {
  int i;
  for (i=0;i<128;i++) {
    dir[i].valid=0;
    memset(dir[i].name,0,255);
    dir[i].inode=-1;
  }
}

void intializeInodes() {
  int i;
  for (i=0;i<128;i++) {
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

  for(i=0;i<128;i++) {
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

  for(i=0;i<128;i++) {
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

void open(char * filename) {
  FILE *fd = fopen(filename,"r");
  fread(blocks,BLOCK_SIZE,NUM_BLOCKS,fd);
  fclose(fd);
}

void close(char * filename) {
  FILE *fd = fopen(filename,"w");
  fwrite(blocks,BLOCK_SIZE,NUM_BLOCKS,fd);
  fclose(fd);
  memset(blocks, 0,NUM_BLOCKS*BLOCK_SIZE);
}

// gets size that is remaining in the disk image
long df() {
  long free_size = 0;
  for(int i = 0; i < NUM_BLOCKS; i++) {
    if(dir[i].valid != 1) {
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
    printf("put error: the File with the name %s does not exist",filename);
  } else {
    FILE *fd = fopen(filename,"r");
    // gets the file size in bytes
    long file_size = buffer.st_size;
    if(file_size >= 10240000 || file_size > df()) {
      // checks if the file size is greater or equal to than 10MB
      // and also if there is enough free space in the disk image
      printf("put error: the File with the name %s is too big",filename);
    } if(filenameSize(filename) > 255) {
      // checks if the filename is greater than 255 characters
      printf("put error: the Filename is too long");
    }
  }
}

void list() {
  int i;
  for(i =0;i<128;i++){
    if(dir[i].valid && inodeList[dir[i].inode].attributes != HIDDEN){
      printf("%6d\t%s",inodeList[dir[i].inode].size,dir[i].name);
    }
  }
}

void attrib() {

}

int main() {
  dir=(struct Directory_Entry*)&blocks[10];
  inodeList =(struct Inode*)&blocks[6];
  freeInodeList=(uint8_t*)&blocks[4];
  freeBlockList=(uint8_t*)&blocks[5];

  createfs("disk.image");
  open("disk.image");

  put("test.txt");
  close("disk.image");
  
  return 0;
}

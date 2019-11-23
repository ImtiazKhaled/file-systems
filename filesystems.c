#include <stdint.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define BLOCK_SIZE 8192
#define NUM_BLOCKS 4226
#define NUM_FILES 128

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

void createfs(char *filename) {
  int i;
  memset(blocks, 0,NUM_BLOCKS*BLOCK_SIZE);
  FILE *fd=fopen(filename,"w");
  fwrite(blocks,BLOCK_SIZE,NUM_BLOCKS,fd);
  intializeDirectory();
  intializeInodeList();
  intializeBlockList();
  intializeInodes();
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

int main() {
  dir=(struct Directory_Entry*)&blocks[0];
  inodeList =(struct Inode*)&blocks[6];
  freeInodeList=(uint8_t*)&blocks[4];
  freeBlockList=(uint8_t*)&blocks[5];

  createfs("disk.image");
  open("disk.image");
  
  dir[0].valid=1;
  dir[1].valid=1;
  dir[2].valid=1;
  int i;
  for(i=0;i<128;i++) {
    printf("%d: %d\n",i,dir[i].valid);
  }

  close("disk.image");
  return 0;
}

// Use copy like this copyFile("secret","NotSoSecret");

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
      ret=i;
      //freeInodeList[i]=0;
      break;
    }
  }
  return ret;
}

int findFreeDirectory() {
  int ret =-1;
  int i;

  for(i=0;i<128;i++) {
    if(dir[i].valid==0) {
      ret=i;
      //dir[i].valid=0;
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
      ret=i;
      //freeBlockList[i]=0;
      break;
    }
  }
  return ret;
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


int copyFile(char * source,int indexB)
{
  int    status;                  
  struct stat buf;            
  status =  stat( source, &buf ); 

  if( status != -1 )
  {
 
    FILE *ifp = fopen ( source, "r" ); 
    if(ifp==NULL)
    {
      return -1;
    }
    printf("Reading %d bytes from %s\n", (int) buf . st_size, source );
 
    int copy_size   = buf . st_size;
    int offset      = 0;               
    int block_index = 0;
 
    while( copy_size > BLOCK_SIZE )
    {

      int freeBlockIndex = findFreeBlock();
      fseek( ifp, offset, SEEK_SET );
 
      
      int bytes  = fread(&freeBlockList[freeBlockIndex], BLOCK_SIZE, 1, ifp );
      inodeList[indexB].blocks[block_index] = freeBlockIndex;
      inodeList[indexB].size += (uint32_t) BLOCK_SIZE;
      //freeBlockList[block_index] = 0;

      if( bytes == 0 && !feof( ifp ) )
      {
        printf("An error occured reading from the input file.\n");
        return -1;
      }
      clearerr( ifp );
      copy_size -= BLOCK_SIZE;
      offset    += BLOCK_SIZE;
      block_index ++;
    }

    if(copy_size > 0)
    {
      int freeBlockIndex = findFreeBlock();
      fseek(ifp, offset, SEEK_SET);
      int bytes  = fread( freeBlockList, copy_size, 1, ifp );
      inodeList[indexB].blocks[block_index] = freeBlockIndex;
      inodeList[indexB].size += (uint32_t) copy_size;
      if( bytes == 0 && !feof( ifp ) )
      {
        printf("An error occured reading from the input file.\n");
        return -1;
      }
      clearerr( ifp );
      copy_size -= BLOCK_SIZE;
      offset    += BLOCK_SIZE;
      block_index ++;
    }
    fclose( ifp );
    return 0;
   }
   return -1;
 }




int writeFile(char * source,int indexB)
{
  
    FILE *ofp;
    ofp = fopen(source, "w");

    int block_index = 0;
    int copy_size   = inodeList[indexB].size;
    int offset      = 0;

    printf("Writing %d bytes to %s\n", copy_size , source );
    while( copy_size > 0 )
    { 

      int num_bytes;

      if( copy_size < BLOCK_SIZE )
      {
        num_bytes = copy_size;
      }
      else 
      {
        num_bytes = BLOCK_SIZE;
      }

      fwrite(&freeBlockList[inodeList[indexB].blocks[block_index]], num_bytes, 1, ofp ); 

      copy_size -= BLOCK_SIZE;
      offset    += BLOCK_SIZE;
      block_index ++;

      fseek( ofp, offset, SEEK_SET );
    }
    fclose( ofp );
 }


int getFileInodeIndex(char * fil)
{
  int i;
  for(i=0;i<128;i++) {
    if(dir[i].valid==1 && strcmp(fil,dir[i].name)==0)
    {
        return i;
    } 
  }
  return -1;
}


void put(char * fileName)
{
  int freeDirIndex = findFreeDirectory();
  int freeInode = findFreeInode();
  //printf("\n\nYay! Found a free directory at index %d and free inode at %d going to set it as used now even though I ain't copying anything!\n",freeDirIndex,freeInode);
  
  dir[freeDirIndex].valid=1;
  dir[freeDirIndex].inode = freeInode;
  strcpy (dir[freeDirIndex].name, fileName);
  freeInodeList[freeInode] = 0;
  inodeList[freeInode].attributes = 0;
  inodeList[freeInode].size  = 0; // This will change later depending on file size;
  int copyCheck = copyFile(fileName,freeInode);
  if(copyCheck==-1)
  {
    printf("\nSomething went wrong when trying to copy, make sure the file exsists!\n");
  }
}

void get(char * fileNameToRead,char * fileToWrite)
{
  int getFileNode = getFileInodeIndex(fileNameToRead);
  if(getFileNode!=-1)
  {
    writeFile(fileToWrite,getFileNode);
  }
  else
  {
    printf("\nSorry, could not find specified file in the file system!\n");
  }
}



int main() {
  dir=(struct Directory_Entry*)&blocks[0];
  inodeList =(struct Inode*)&blocks[6];
  freeInodeList=(uint8_t*)&blocks[4];
  freeBlockList=(uint8_t*)&blocks[5];

  //createfs("disk.image");
  open("disk.image");
  
  int i;
  for(i=0;i<128;i++)
  {
    if(dir[i].valid==1)
    {
      printf("\n%d %d\n",dir[i].inode,inodeList[dir[i].inode].size);
    }
  }

  //put("secret2");
  //get("secret","noSoSecret");

  close("disk.image");
  return 0;
}

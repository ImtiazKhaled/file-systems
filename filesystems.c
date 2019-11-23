/*
  Imtiaz Mujtaba Khaled
  1001551928

  Sam Thomas

*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>


#define BLOCK_SIZE 8192
#define NUM_BLOCKS 4226
#define NUM_FILES 128
#define MAX_COMMAND_SIZE 255     
#define MAX_NUM_ARGUMENTS 10
#define HISTORY_NUM 15      
#define SHELL_NAME "msh>"       
#define WHITESPACE " \t\n"

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
  struct stat buffer;
  if(stat(filename, &buffer) != 0) {
    printf("open error: File not found\n");
  } else {
    FILE *fd = fopen(filename,"r");
    fread(blocks,BLOCK_SIZE,NUM_BLOCKS,fd);
    fclose(fd);
  }
}

// write the block data into a file
void closeFile(char * filename) {
  struct stat buffer;
  if(stat(filename, &buffer) != 0) {
    printf("close error: File not found\n");
  } else {
    FILE *fd = fopen(filename,"w");
    fwrite(blocks,BLOCK_SIZE,NUM_BLOCKS,fd);
    fclose(fd);
    memset(blocks, 0,NUM_BLOCKS*BLOCK_SIZE);
  }
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
  int i, flag=1;
  for(i = 0; i<NUM_FILES; i++){
    if(dir[i].valid == 1 && (inodeList[dir[i].inode].attributes != HIDDEN && inodeList[dir[i].inode].attributes != BOTH)){
      // checks to see if the file in the file system
      // is either hidden or both hidden and read-only
      flag=0;
      printf("%6d\t%s\n",inodeList[dir[i].inode].size,dir[i].name);
    }
  }
  if(flag) {
      printf("list: No files found.\n");
  }
}

void listWithHidden() {
  int i, flag=1;
  for(i = 0; i<NUM_FILES; i++){
    if(dir[i].valid == 1){
      // checks to see if the file in the file system
      // is either hidden or both hidden and read-only
      flag=0;
      printf("%6d\t%s\n",inodeList[dir[i].inode].size,dir[i].name);
    }
  }
  if(flag) {
      printf("list: No files found.\n");
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

// This function handels all the shell operations 
void shell_operations(char * cmd_str){
  
  int i;
  // Ignores user input if it is a blank line
  if(strcmp(cmd_str, "\n") == 0)  return;
  
  /* Parse input */
  char *token[MAX_NUM_ARGUMENTS];
  int   token_count = 0;         
                                                         
  // Pointer to point to the token
  // parsed by strsep
  char *arg_ptr;                                         
                                                         
  char *working_str  = strdup( cmd_str );

  // we are going to move the working_str pointer so
  // keep track of its original value so we can deallocate
  // the correct amount at the end
  char *working_root = working_str;
  
  // Stores the pid for the current process
  pid_t curr_pid;
  char *curr_command = strdup(working_root);
  curr_command = strtok(curr_command, "\n");  // Removes trailing new line from input
  
  // Tokenize the input stringswith whitespace used as the delimiter
  while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
            (token_count<MAX_NUM_ARGUMENTS)){
    token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
    if( strlen( token[token_count] ) == 0 ){

      token[token_count] = NULL;
    }
      token_count++;

  }

  if(strcmp(token[0],"exit") == 0 || strcmp(token[0],"quit") == 0){
  // Exit when user input is exit or quit        
    exit(0);
  } else if(strcmp(token[0],"createfs") == 0) {
    // creates a new file system with the user input
    if(token_count == 2) {
      printf("createfs: File not found\n");
    } else {
      createfs(token[1]);
    }
  } else if(strcmp(token[0],"open") == 0) {
    // opens a previously made file system with the user input
    if(token_count == 2) {
      printf("open: File not found\n");
    } else {
      open(token[1]);
    }
  } else if(strcmp(token[0],"close") == 0) {
    // closes a previously made file system with the user input
    if(token_count == 2) {
      printf("close: File not found\n");
    } else {
      closeFile(token[1]);
    }
  } else if(strcmp(token[0],"df") == 0) {
    // prints the amount of free space in bytes remaining
    printf("%ld bytes free.\n",df());
  } else if(strcmp(token[0],"list") == 0) {
    // lists all the files in the file system that are not hidden
    if(token_count == 2) {
        list();
    } else {
      if(strcmp(token[1],"-h") == 0) {
        // if -h flag is given, then hidden files are also printed
        listWithHidden();
      } else {
        list();
      }
    }
  } else if(strcmp(token[0],"del") == 0) {
    // lists all the files in the file system that are not hidden
    if(token_count == 2) {
      printf("del error: File not found\n");
    } else {
      del(token[1]);
    }
  } else if(strcmp(token[0],"attrib") == 0) {
    // lists all the files in the file system that are not hidden
    if(token_count < 4) {
      printf("attrib error: Invalid command\n");
    } else {
      attrib(token[1],token[2]);
    }
  } else if(strcmp(token[0],"put") == 0) { 
    // puts a file from the current directory into the file system
    if(token_count == 2) {
      printf("put: File not found\n");
    } else {
      put(token[1]);
    }
  }
    
}



int main() {
  dir=(struct Directory_Entry*)&blocks[0];
  inodeList =(struct Inode*)&blocks[6];
  freeInodeList=(uint8_t*)&blocks[4];
  freeBlockList=(uint8_t*)&blocks[5];
  
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  
  while( 1 ){      
    // Print out the msh prompt
    printf("%s ", SHELL_NAME);

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );
    shell_operations(cmd_str);
  }

  free(cmd_str);
  
  // // put into fs
  // put("test.txt");
  
  // // change file attributes
  // attrib("+h","hi.there");
  // attrib("+r","hi.there");
  
  // // change file attributes
  // attrib("+h","hi.there");
  
  return 0;
}

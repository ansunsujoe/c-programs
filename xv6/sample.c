#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <stdbool.h>

#include "types.h"
#include "fs.h"

#define BLOCK_SIZE (BSIZE)


int
main(int argc, char *argv[])
{
  int r,i,n,fsfd;
  char *addr;
  struct dinode *dip;
  struct superblock *sb;
  struct dirent *de;
  uchar *bitmap;

  if(argc < 2){
    fprintf(stderr, "Usage: sample fs.img ...\n");
    exit(1);
  }

  // Open the file system image
  fsfd = open(argv[1], O_RDONLY);
  if(fsfd < 0){
    perror(argv[1]);
    exit(1);
  }

  /* Dont hard code the size of file. Use fstat to get the size */
  // Change this later
  addr = mmap(NULL, 524248, PROT_READ, MAP_PRIVATE, fsfd, 0);
  if (addr == MAP_FAILED){
	perror("mmap failed");
	exit(1);
  }
  /* read the super block */
  sb = (struct superblock *) (addr + 1 * BLOCK_SIZE);
  printf("fs size %d, no. of blocks %d, no. of inodes %d \n", sb->size, sb->nblocks, sb->ninodes);

  /* read the inodes */
  dip = (struct dinode *) (addr + IBLOCK((uint)0)*BLOCK_SIZE); 
  printf("begin addr %p, begin inode %p , offset %d \n", addr, dip, (char *)dip -addr);
  printf("Root inode  size %d links %d type %d \n", dip[ROOTINO].size, dip[ROOTINO].nlink, dip[ROOTINO].type);
  
  /* read the bitmap */
  bitmap = (uchar *) (addr + BBLOCK(0, sb->ninodes) * BLOCK_SIZE);

  // get the address of root dir 
  de = (struct dirent *) (addr + (dip[ROOTINO].addrs[0])*BLOCK_SIZE);

  // TEST 3: If the root directory is null, then throw error
  if (dip[ROOTINO] == NULL) {
    printf("ERROR: root directory does not exist.\n");
    exit(1);
  }

  // TEST 3: If the root directory's type is not DIR = 1, then throw an error
  if (dip[ROOTINO].type != 1) {
    printf("ERROR: root directory does not exist.\n");
    exit(1);
  }

  // print the entries in the first block of root dir 
  n = dip[ROOTINO].size/sizeof(struct dirent);

  // Variables for parent and current directory configs
  bool parentFound = false;
  bool currentFound = false;

  // Iterate through root directory to check for parent directory and current (.. and .)
  for (i = 0; i < n; i++,de++) {
 	  printf(" inum %d, name %s ", de->inum, de->name);
  	printf("inode  size %d links %d type %d \n", dip[de->inum].size, dip[de->inum].nlink, dip[de->inum].type);

    // Test 3: If parent directory is not itself, then error
    if (de->name == "..") {
      if (de->inum != ROOTINO) {
        printf("ERROR: root directory does not exist.\n");
        exit(1);
      }
      else {
        parentFound = true;
        break;
      }
    }
  }

  // If we cannot find the parent in the root directory, then throw an error
  if (parentFound == false) {
    printf("ERROR: root directory does not exist.\n");
    exit(1);
  }

  /* Test 1: Bad Inode */

  // Variables needed
  uint iIndex;
  struct dinode *inodeBlock;

  printf("Beginning test 1\n");

  // Iterate through inode numbers
  for (i = 0; i < sb->ninodes; i++) {
    
    // Find exact location of the inode number
    inodeBlock = (struct dinode *) (addr + IBLOCK((uint)i) * BLOCK_SIZE);
    iIndex = i % IPB;
    
    // If inode is unallocated, make sure size is 0 and type is 0 
    if (inodeBlock[iIndex].type == 0) {
      if (inodeBlock[iIndex].size != 0) {
        printf("ERROR: bad inode.\n");
	      exit(1);
      }
    }

    // If inode is not type 1, 2, or 3, then we raise an error for bad inode.
    else if (inodeBlock[iIndex].type != 1 && inodeBlock[iIndex].type != 2 && inodeBlock[iIndex].type != 3) {
      printf("ERROR: bad inode.\n");
      exit(1);
    }

    // TEST 4: Checking if a directory is properly formatted
    if (inodeBlock[iIndex].type == 1) {

      // get the address of current directory
      de = (struct dirent *) (addr + (inodeBlock[iIndex].addrs[0])*BLOCK_SIZE);
      n = inodeBlock[iIndex].size / sizeof(struct dirent);

      // Flags used to determine if the parent and current are found
      parentFound = false;
      currentFound = false;

      // Iterate through root directory to check for parent directory and current (.. and .)
      for (i = 0; i < n; i++, de++) {
 	      printf(" inum %d, name %s ", de->inum, de->name);
  	    printf("inode  size %d links %d type %d \n", dip[de->inum].size, dip[de->inum].nlink, dip[de->inum].type);

        // Check if we have crossed paths with the parent entry
        if (de->name == "..") {
          parentFound = true;
        }

        // Check if the current directory entry is in order
        if (de->name == ".") {
          // TEST 4: If the current directory does not point to the current inode
          // then we error out
          if (de->inum != i) {
            printf("ERROR: directory not properly formatted.\n");
            exit(1);
          }
          else {
            currentFound = true;
          }
        }

        // Early break if we found both parent and current
        if (parentFound == true && currentFound == true) {
          break;
        }

      }
      // TEST 4: Check the flags - if current or parent is false, then it does
      // not exist and we error out.
      if (parentFound == false || currentFound == false) {
        printf("ERROR: directory not properly formatted.\n");
        exit(1);
      }

    }

    // Iterate through the inode addresses
    int j = 0;
    for (j = 0; j < NDIRECT + 1; j++) {
      
    }
  }

  // Exit
  exit(0);

}


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
  struct stat st;

  if(argc < 2){
    fprintf(stderr, "Usage: fcheck <file_system_image>\n");
    exit(1);
  }

  // Open the file system image
  fsfd = open(argv[1], O_RDONLY);
  if (fsfd < 0){
    fprintf(stderr, "image not found.\n");
    exit(1);
  }

  // Get stats about the filesystem (especially the size is needed)
  if (fstat(fsfd, &st) < 0){
    fprintf(stderr, "Cannot get stats for file.\n");
    close(fsfd);  // Close called on the file descriptor (done in each error case)
    exit(1);
  }

  /* Dont hard code the size of file. Use fstat to get the size */
  // Change this later
  addr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fsfd, 0);
  if (addr == MAP_FAILED){
	  perror("mmap failed");
    close(fsfd);
  	exit(1);
  }
  /* read the super block */
  sb = (struct superblock *) (addr + 1 * BLOCK_SIZE);
  printf("fs size %d, no. of blocks %d, no. of inodes %d \n", sb->size, sb->nblocks, sb->ninodes);

  /* read the inodes */
  dip = (struct dinode *) (addr + (IBLOCK((uint)0))*BLOCK_SIZE); 
  printf("begin addr %p, begin inode %p , offset %d \n", addr, dip, (char *)dip -addr);
  printf("Root inode  size %d links %d type %d \n", dip[ROOTINO].size, dip[ROOTINO].nlink, dip[ROOTINO].type);
  
  /* read the bitmap */
  // bitmap = (uchar *) (addr + BBLOCK(0, sb->ninodes) * BLOCK_SIZE);
  bitmap = (uchar *) (addr + (sb->ninodes / IPB + 3) * BLOCK_SIZE);

  // get the address of root dir 
  de = (struct dirent *) (addr + (dip[ROOTINO].addrs[0])*BLOCK_SIZE);

  // TEST 3: If the root directory is null, then throw error
  if (dip[ROOTINO].size == 0) {
    fprintf(stderr, "ERROR: root directory does not exist.\n");
    close(fsfd);
    exit(1);
  }

  // TEST 3: If the root directory's type is not DIR = 1, then throw an error
  if (dip[ROOTINO].type != 1) {
    fprintf(stderr, "ERROR: root directory does not exist.\n");
    close(fsfd);
    exit(1);
  }

  // print the entries in the first block of root dir 
  n = dip[ROOTINO].size/sizeof(struct dirent);

  // Variables for parent and current directory configs
  bool parentFound = false;
  bool currentFound = false;

  // Iterate through root directory to check for parent directory and current (.. and .)
  for (i = 0; i < n; i++,de++) {

    // Test 3: If parent directory is not itself, then error
    if (strcmp(de->name, "..") == 0) {
      if (de->inum != ROOTINO) {
        fprintf(stderr, "ERROR: root directory does not exist.\n");
        close(fsfd);
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
    fprintf(stderr, "ERROR: root directory does not exist.\n");
    close(fsfd);
    exit(1);
  }

  /* Test 1: Bad Inode */

  // Variables needed
  uint iIndex;
  struct dinode *inodeBlock;
  int bmchecker[sb->size];
  int inodeChecker[sb->ninodes];
  int refChecker[sb->ninodes];
  int isDirectory[sb->ninodes];

  int j = 0;
  uint currentAddress;
  uint* indirectAddress;

  // Calculate where the valid addresses for data blocks starts and ends
  uint dataBlockStart = 3 + (sb->ninodes / IPB) + (sb->size / BPB) + 1;
  uint dataBlockEnd = dataBlockStart + sb->nblocks - 1;

  // Populate the bitmap checker with values from bitmap for each block
  for (i = dataBlockStart; i < sb->size; i++) {
    bmchecker[i] = (int) bitmap[i/8] >> (i % 8) & 1;
  }

  // Populate inode checker and ref checker and isDirectory with zero values
  for (i = 0; i < sb->ninodes; i++) {
    inodeChecker[i] = 0;
    refChecker[i] = 0;
    isDirectory[i] = 0;
  }

  printf("Beginning test 1\n");

  // Iterate through inode numbers
  for (i = 0; i < sb->ninodes; i++) {
    
    // Find exact location of the inode number
    inodeBlock = (struct dinode *) (addr + IBLOCK((uint)i) * BLOCK_SIZE);
    iIndex = i % IPB;
    
    // If inode is unallocated, make sure size is 0 and type is 0 and links is 0
    if (inodeBlock[iIndex].type == 0 && inodeBlock[iIndex].nlink == 0) {
      if (inodeBlock[iIndex].size != 0) {
        fprintf(stderr, "ERROR: bad inode.\n");
        close(fsfd);
	      exit(1);
      }
    }

    // If inode is not type 1, 2, or 3, then we raise an error for bad inode.
    else if (inodeBlock[iIndex].type != 1 && inodeBlock[iIndex].type != 2 && inodeBlock[iIndex].type != 3) {
      fprintf(stderr, "ERROR: bad inode.\n");
      close(fsfd);
      exit(1);
    }

    // Note that the inode is in use
    else if (inodeChecker[i] % 2 == 0) {
      inodeChecker[i] += 1;
    }

    // Iterate through the inode addresses -> this is for DIRECTS ONLY
    for (j = 0; j < NDIRECT + 1; j++) {
      // Test 2: Check if each address in the addrs[] is valid. 
      // If not, throw an error message
      currentAddress = inodeBlock[iIndex].addrs[j];

      // If the address is not null (unallocated)
      if (currentAddress != 0) {
        // TEST 2: If the address is out of the data block range
        if (currentAddress < dataBlockStart || currentAddress > dataBlockEnd) {
          // Check if it is a direct or indirect address that is violating:
          // Pick which error to throw
          // Case 1: Indirect error
          if (j == NDIRECT) {
            fprintf(stderr, "ERROR: bad indirect address in inode.\n");
            close(fsfd);
            exit(1);
          }
          // Case 2: Direct error
          else {
            fprintf(stderr, "ERROR: bad direct address in inode.\n");
            close(fsfd);
            exit(1);
          }
        }

        // Check the validity of the bitmap using our bitmap checker
        // Used for tests 5-8
        // In the bitmap array: 1 - block used but not accouted for
        //                      2 - block used and accounted for
        //                      0 - block not used

        // Case 1: Inode has the address but bitmap says it is free
        if (bmchecker[currentAddress] == 0) {
          fprintf(stderr, "ERROR: address used by inode but marked free in bitmap.\n");
          close(fsfd);
          exit(1);
        }
        // Case 2: Inode has an address that has already been accounted for by
        // a previous inode
        else if (bmchecker[currentAddress] == 2) {
          // An indirect block is throwing the duplicate error
          if (j == NDIRECT) {
            fprintf(stderr, "ERROR: indirect address used more than once.\n");
            close(fsfd);
            exit(1);
          }
          // A direct block is throwing the duplicate error
          else {
            fprintf(stderr, "ERROR: direct address used more than once.\n");
            close(fsfd);
            exit(1);
          }
        }
        // Case 3: Inode has an address that has not been accounted for before
        // and we will mark that address in bmchecker as accounted for
        else if (bmchecker[currentAddress] == 1) {
          bmchecker[currentAddress] = 2;
        }
      }
    }

    // Iterate through the inode addresses -> this is for INDIRECT BLOCK
    currentAddress = inodeBlock[iIndex].addrs[NDIRECT];
    indirectAddress = (uint*) (addr + currentAddress * BLOCK_SIZE);
    for (j = 0; j < NINDIRECT; j++, indirectAddress++) {
      // To be continued...
      
      // If the address is not null (unallocated)
      if (*indirectAddress != 0) {
        // TEST 2: If the address is out of the data block range
        if (*indirectAddress < dataBlockStart || *indirectAddress > dataBlockEnd) {
          fprintf(stderr, "ERROR: bad indirect address in inode.\n");
          close(fsfd);
          exit(1);
        }

        // Check the validity of the bitmap using our bitmap checker
        // Used for tests 5-8
        // In the bitmap array: 1 - block used but not accouted for
        //                      2 - block used and accounted for
        //                      0 - block not used

        // Case 1: Inode has the address but bitmap says it is free
        if (bmchecker[*indirectAddress] == 0) {
          fprintf(stderr, "ERROR: address used by inode but marked free in bitmap.\n");
          close(fsfd);
          exit(1);
        }
        // Case 2: Inode has an address that has already been accounted for by
        // a previous inode
        else if (bmchecker[*indirectAddress] == 2) {
          // An indirect block is throwing the duplicate error
          if (j == NDIRECT) {
            fprintf(stderr, "ERROR: indirect address used more than once.\n");
            close(fsfd);
            exit(1);
          }
          // A direct block is throwing the duplicate error
          else {
            fprintf(stderr, "ERROR: direct address used more than once.\n");
            close(fsfd);
            exit(1);
          }
        }
        // Case 3: Inode has an address that has not been accounted for before
        // and we will mark that address in bmchecker as accounted for
        else if (bmchecker[*indirectAddress] == 1) {
          bmchecker[*indirectAddress] = 2;
        }
      }
    }


    // TEST 4: Checking if a directory is properly formatted
    if (inodeBlock[iIndex].type == 1) {

      // Let's mark that this is a directory
      isDirectory[i] = 1;

      // get the address of current directory
      de = (struct dirent *) (addr + (inodeBlock[iIndex].addrs[0])*BLOCK_SIZE);
      n = inodeBlock[iIndex].size / sizeof(struct dirent);

      // Flags used to determine if the parent and current are found
      parentFound = false;
      currentFound = false;

      // Iterate through all directories to check for parent directory and current (.. and .)
      int k;
      printf("This is directory for inode %d\n", i);
      for (k = 0; k < n; k++, de++) {
        if (de->inum != 0 || dip[de->inum].size != 0) {
          printf(" inum %d, name %s ", de->inum, de->name);
  	      printf("inode  size %d links %d type %d \n", dip[de->inum].size, dip[de->inum].nlink, dip[de->inum].type);
        }

        // Check if we have crossed paths with the parent entry
        if (strcmp(de->name, "..") == 0) {
          parentFound = true;
        }

        // Check if the current directory entry is in order
        if (strcmp(de->name, ".") == 0) {
          // TEST 4: If the current directory does not point to the current inode
          // then we error out
          if (de->inum != i) {
            fprintf(stderr, "ERROR: directory not properly formatted.\n");
            close(fsfd);
            exit(1);
          }
          else {
            currentFound = true;
          }
        }

        // Record in the inode checker that the inum was referenced
        if (inodeChecker[de->inum] < 2) {
          inodeChecker[de->inum] += 2;
        }

        // Record in the ref checker that the inum was referenced, but not if it's a .. on a directory
        if (strcmp(de->name, "..") && strcmp(de->name, ".")) {
          refChecker[de->inum]--;
        }
      }
      // TEST 4: Check the flags - if current or parent is false, then it does
      // not exist and we error out.
      if (parentFound == false || currentFound == false) {
        fprintf(stderr, "ERROR: directory not properly formatted.\n");
        close(fsfd);
        exit(1);
      }
    }

    // Add the number of references to the ref checker
    // The purpose is that when a directory references the file,
    // we will decrement the ref checker every time and should
    // end up with 0 for each address in a consistent file system
    refChecker[i] += inodeBlock[iIndex].nlink;
    // In the case of a directory, we're going to set refChecker to 1
    if (inodeBlock[iIndex].type == 1) {
      refChecker[i] = 0;
    }
    // In the case of a root directory, there are no inodes referencing this
    // inode, so subtract 1
    // if (i == 1) {
    //   refChecker[i]--;
    // }
  }

  // TEST 6: If bitmap checker still has used blocks not accounted for
  // in any inodes addrs[] field, then flag an error
  for (i = dataBlockStart; i < dataBlockStart + sb->nblocks + 1; i++) {
    if (bmchecker[i] == 1) {
      fprintf(stderr, "ERROR: bitmap marks block in use but it is not in use.\n");
      close(fsfd);
      exit(1);
    }
  }

  // TEST 9-10: Let's see if the inode checker's final state is as desired
  // Possible states for each inode:
  //    0 - not in use and not referenced in directory
  //    1 - in use but not referenced in directory (INCONSISTENT!)
  //    2 - not in use but referenced in directory (INCONSISTENT!)
  //    3 - in use and referenced in directory 
  for (i = 1; i < sb->ninodes; i++) {
    if (inodeChecker[i] == 1) {
      fprintf(stderr, "ERROR: inode marked use but not found in a directory.\n");
      close(fsfd);
      exit(1);
    }
    else if (inodeChecker[i] == 2) {
      fprintf(stderr, "ERROR: inode referred to in directory but marked free.\n");
      close(fsfd);
      exit(1);
    }
  }

  // TEST 11-12: Make sure that the number of links (refs) to a regular file
  // that is recorded in inode is equal to actuality in the directory entries
  for (i = 1; i < sb->ninodes; i++) {
    // If the value in refChecker is anything other than 0
    // given it's a regular file, then we have reference error (Test 11)
    // printf("Refchecker for inode %d is %d, directory is %d\n", i, refChecker[i], isDirectory[i]);
    if (refChecker[i] != 0 && isDirectory[i] == 0) {
      fprintf(stderr, "ERROR: bad reference count for file.\n");
      close(fsfd);
      exit(1);
    }
    // If the inode is directory and the refChecker is not -1
    // Then throw an error (TEST 12)
    else if (isDirectory[i] == 1 && refChecker[i] != 0) {
      fprintf(stderr, "ERROR: directory appears more than once in file system.\n");
      close(fsfd);
      exit(1);
    }
  }

  // Exit
  exit(0);
}


/*
  Copyright (C) 2016 Hung-Wei Tseng, Ph.D. <hungwei_tseng@ncsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  You may extend this file if necessary  
*/


#include <sys/stat.h>

#define SUPER_BLOCK_SIZE 1
#define SUPER_BLOCK_MIN 0
#define SUPER_BLOCK_MAX (SUPER_BLOCK_MIN+SUPER_BLOCK_SIZE)

#define INODE_BLOCK_SIZE 10
#define INODE_BLOCK_MIN (SUPER_BLOCK_MAX+1)
#define INODE_BLOCK_MAX (INODE_BLOCK_MIN+INODE_BLOCK_SIZE)

#define DATA_BLOCK_SIZE 16000
#define DATA_BLOCK_MIN (INODE_BLOCK_MAX+1)
#define DATA_BLOCK_MAX (DATA_BLOCK_MIN+DATA_BLOCK_SIZE)

#define MAX_NAME 50
#define DATA_BLOCK_SIZE 8192

typdef struct {
	struct  stat fstat;
	char 		 fileName[MAX_NAME];
	char		 dirName[MAX_NAME];
	char		 pad[2];	
} tInodeInfo;

#define BLOCK_ENTRIES_NUM (BLOCK_SIZE/sizeof(tInodeInfo))


struct nphfuse_state {
    FILE *logfile;
    char *device_name;
    int devfd;
};

/*
  Copyright (C) 2016 Hung-Wei Tseng, Ph.D. <hungwei_tseng@ncsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  You may extend this file if necessary  
*/

#include <sys/stat.h>
#include <stdint.h>

#define SUPER_BLOCK_COUNT 1
#define SUPER_BLOCK_START 1
#define SUPER_BLOCK_END 2

#define INODE_BLOCK_COUNT 998
#define INODE_BLOCK_START 2
#define INODE_BLOCK_END 1000

#define DATA_BLOCK_COUNT 10000
#define DATA_BLOCK_START 1000
#define DATA_BLOCK_END 11000

#define FILE_MAX 128
#define BLOCK_CAPACITY 8192
#define DIR_MAX 224

typedef struct i_node{
	struct  stat fstat;
	char 		 file_name[FILE_MAX];
  char		 dir_name[DIR_MAX];
  uint64_t offset;
  long int		 pad;	
} i_node;

#define INODE_NUM (BLOCK_CAPACITY/sizeof(i_node)) //16

struct nphfuse_state {
    FILE *logfile;
    char *device_name;
    int devfd;
};

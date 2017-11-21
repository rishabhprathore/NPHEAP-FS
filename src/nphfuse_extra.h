/*
  Copyright (C) 2016 Hung-Wei Tseng, Ph.D. <hungwei_tseng@ncsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  You may extend this file if necessary  
*/


#include <sys/stat.h>

#define SUPER_BLOCK_COUNT 1
#define SUPER_BLOCK_START 0
#define SUPER_BLOCK_END 1

#define INODE_BLOCK_COUNT 50
#define INODE_BLOCK_START 1
#define INODE_BLOCK_END 51

#define DATA_BLOCK_COUNT 10000
#define DATA_BLOCK_START 51
#define DATA_BLOCK_END 10051

#define FILE_MAX 32
#define BLOCK_CAPACITY 8192
#define DIR_MAX 64

typdef struct {
	struct  stat fstat;
	char 		 file_name[FILE_MAX];
  char		 dir_name[FILE_MAX];
  uint64_t offset;
  long int		 pad;	
} i_node;

#define INODE_NUM (BLOCK_CAPACITY/sizeof(i_node))


struct nphfuse_state {
    FILE *logfile;
    char *device_name;
    int devfd;
};

/*
  Copyright (C) 2016 Hung-Wei Tseng, Ph.D. <hungwei_tseng@ncsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  You may extend this file if necessary  
*/

#include <sys/stat.h>
#include <stdint.h>

#define FILE_MAX 128
#define DIR_MAX 224

typedef struct i_node{
  struct  stat fstat;
  uint64_t offset;
  char 		 file_name[FILE_MAX];
  char		 dir_name[DIR_MAX];
  long int		 pad;	
} i_node;

struct nphfuse_state {
    FILE *logfile;
    char *device_name;
    int devfd;
};

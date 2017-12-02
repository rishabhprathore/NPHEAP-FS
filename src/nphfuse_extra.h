// Project 3: Aasheesh Tandon, atandon; Rishabh Rathore, rrathor;

/*
  Copyright (C) 2016 Hung-Wei Tseng, Ph.D. <hungwei_tseng@ncsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  You may extend this file if necessary  
*/

// Project 3: Aasheesh Tandon, atandon; Rishabh Rathore, rrathor;

#include <sys/stat.h>
#include <stdint.h>

typedef struct i_node
{
  struct stat fstat;
  uint64_t offset;
  char file_name[128];
  char dir_name[224];
  long int pad;
} i_node;

struct nphfuse_state
{
  FILE *logfile;
  char *device_name;
  int devfd;
};
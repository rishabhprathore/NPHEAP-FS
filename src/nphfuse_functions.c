/*
  NPHeap File System
  Copyright (C) 2016 Hung-Wei Tseng, Ph.D. <hungwei_tseng@ncsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  This code is derived from function prototypes found /usr/include/fuse/fuse.h
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  His code is licensed under the LGPLv2.
  A copy of that code is included in the file fuse.h
  
  The point of this FUSE filesystem is to provide an introduction to
  FUSE.  It was my first FUSE filesystem as I got to know the
  software; hopefully, the comments in this code will help people who
  follow later to get a gentler introduction.

*/

#include "nphfuse.h"
#include <npheap.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

int npheap_fd = 0;
uint64_t inode_num = 2;
uint64_t data_offset = 1000;

uint8_t *data_array[10999];
uint64_t data_next[10000];

///////////////////////////////////////////////////////////
//
// Prototypes for all these functions, and the C-style comments,
// come from /usr/include/fuse.h
//
/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern struct nphfuse_state *nphfuse_data;

int CanUseInode(i_node *inode_data)
{
    if (inode_data == NULL)
        return 0;
    if ((getuid() == 0) || (getgid() == 0))
        return 1;
    if (inode_data->fstat.st_uid == getuid() ||
        (inode_data->fstat.st_gid == getgid()))
        return 1;
    return 0;
}

void split_func(char *local_pathname)
{

    char *ts1 = strdup(local_pathname);
    char *ts2 = strdup(local_pathname);
    char *dir = dirname(ts1);
    char *filename = basename(ts2);
    //call your funtion here in the place of the three lines below
    printf("\nDIRECTORY: %s", dir);
    printf("\nFILENAME: %s", filename);
    printf("\n-------------**-------------");
    return;
}

void path_name(char *fullpath)
{

    char *pos = NULL;
    char *temp = NULL;
    char *local_pathname = NULL;
    printf("\nfullpath passed as INPUT is: %s\n", fullpath);
    for (temp = fullpath; temp <= fullpath + strlen(fullpath); temp++)
    {
        if (*temp == '/')
        {
            for (pos = temp + 1; pos <= fullpath + strlen(fullpath); pos++)
            {
                if (*pos == '/' || *pos == '\0')
                {
                    local_pathname = malloc(pos - fullpath + 1);
                    strncpy(local_pathname, fullpath, pos - fullpath);
                    local_pathname[pos - fullpath] = '\0';
                    split_func(local_pathname);
                    break;
                }
            }
        }
    }
    free(local_pathname);
}


int resolve_path(const char *path, char *dir, char *file)
{
    log_msg("\nInside resolve_path\n");
    char *string = NULL;
    char *ptr = NULL;
    char *prev = NULL;

    if (!path || !dir || !file)
    {
        return 1;
    }
    memset(dir, 0, 224);
    memset(file, 0, 128);

    if (!strcmp(path, "/"))
    {
        strcpy(dir, "/");
        strcpy(file, "/");
        printf("[%s]: dir:%s, file:%s\n", __func__, dir, file);
        return 0;
    }

    string = strdup(path);
    if (!string)
    {
        printf("failed to allocate memory to string\n");
        return 1;
    }

    ptr = strtok(string, "/");
    if (!ptr)
    {
        free(string);
        return 1;
    }

    prev = ptr;
    while ((ptr = strtok(NULL, "/")) != NULL)
    {
        strncat(dir, "/", 224);
        strncat(dir, prev, 128);
        prev = ptr;
    }

    if (dir[0] == '\0')
    {
        strcpy(dir, "/");
    }
    strncpy(file, prev, 128);

    printf("[%s]: dir:%s, file:%s\n", __func__, dir, file);
    free(string);
    return 0;
}

static i_node *get_root_inode(void)
{
    i_node *root_inode = NULL;
    i_node *test_inode = NULL;
    log_msg("\nget_root_inode()  called %d", npheap_getsize(npheap_fd, 2));
    //root_inode = (i_node *)npheap_alloc(npheap_fd, 2,npheap_getsize(npheap_fd, 2));

    root_inode = (i_node *)data_array[2];
    test_inode = &root_inode[0];
    log_msg("\ncheck in test_inode\n");
    log_msg("\n test_inode links- %d, size - %d",
            test_inode->fstat.st_nlink, test_inode->fstat.st_size);
    return test_inode;
}

static i_node *get_inode(const char *path)
{
    i_node *inode_data = NULL;
    char dir_name[224];
    char file_name[128];
    __u64 offset = 0;
    int i = 0;

    if (strcmp(path, "/") == 0)
    {
        log_msg("\ncalling get_root_inode from get_inode()\n");
        return get_root_inode();
    }

    if (resolve_path(path, dir_name, file_name) != 0)
    {
        log_msg("\ndirfinename failed!!!\n");
        return NULL;
    }

    for (offset = 2; offset < 1000; offset++)
    {
        inode_data = (i_node *)data_array[offset];

        if (inode_data == 0)
        {
            return NULL;
        }

        for (i = 0; i < 16; i++)
        {
            if ((strcmp(inode_data[i].dir_name, dir_name) == 0) &&
                (strcmp(inode_data[i].file_name, file_name) == 0))
            {
                return &inode_data[i];
            }
        }
    }

    log_msg("\n get_inode returning NULL\n");
    return NULL;
}

static void npheap_fs_init(void)
{
    uint64_t offset = 0;
    uint8_t *block_data = NULL;
    i_node *inode_data = NULL;
    i_node *root_inode = NULL;

    npheap_fd = open(nphfuse_data->device_name, O_RDWR);
    // allocate offset 0 in npheap for superblock
    log_msg("\n npheap fd  %d\n", npheap_fd);
    memset(data_array, 0, sizeof(uint8_t *) * 10999);
    memset(&data_next, 0, sizeof(data_next));
    if (npheap_getsize(npheap_fd, 1) == 0)
    {
        log_msg("\n inside superblock allocation\n");
        block_data = npheap_alloc(npheap_fd, 1, 8192);
        if (block_data == NULL)
        {
            printf("Failed to allocate npheap memory to offset: 1");
            return;
        }
        memset(block_data, 0, npheap_getsize(npheap_fd, 1));
    }
    else
    {
        block_data = npheap_alloc(npheap_fd, offset,
                                  npheap_getsize(npheap_fd, offset));
    }
    data_array[1] = block_data;
    log_msg("\n Superblock size %d\n", npheap_getsize(npheap_fd, 1));

    for (offset = 2; offset < 1000; offset++)
    {
        if (npheap_getsize(npheap_fd, offset) == 0)
        {
            block_data = npheap_alloc(npheap_fd, offset, 8192);
            memset(block_data, 0, npheap_getsize(npheap_fd, offset));
            log_msg("\n inode size for offset: %d-> %d\n",
                    offset, npheap_getsize(npheap_fd, offset));
        }
        else
            block_data = npheap_alloc(npheap_fd, offset,
                                      npheap_getsize(npheap_fd, offset));

        data_array[offset] = block_data;
    }
    //get info of root directory inode

    root_inode = get_root_inode();
    
    log_msg("\nnphfuse_fs_init()  1\n");
    strcpy(root_inode->dir_name, "/");
    log_msg("\nnphfuse_fs_init()  2\n");
    strcpy(root_inode->file_name, "/");
    root_inode->fstat.st_ino = inode_num++;
    root_inode->fstat.st_mode = S_IFDIR | 0755;
    root_inode->fstat.st_nlink = 2;
    root_inode->fstat.st_size = 8192;
    root_inode->fstat.st_uid = getuid();
    root_inode->fstat.st_gid = getgid();
    return;
}

int nphfuse_getattr(const char *path, struct stat *stbuf)
{
    i_node *inode_data = NULL;
    log_msg("\ninside get_attr for path %s\n", path);
    inode_data = get_inode(path);
    if (inode_data == NULL)
    {
        log_msg("\ngetattr returning ENOENT\n");
        return -ENOENT;
    }
    memcpy(stbuf, &inode_data->fstat, 144);
    return 0;
}

/** Read the target of a symbolic link */
int nphfuse_readlink(const char *path, char *link, size_t size)
{
    log_msg("\nInside readlink\n");
    return -1;
}

/* Helper function for mknod(). Assigns values to i_node struct's fstat parameters*/
void mknod_fstat_helper(i_node *temp_node, mode_t mode, dev_t dev)
{

    struct timeval day_tm;

    temp_node->fstat.st_ino = inode_num++;
    temp_node->fstat.st_mode = mode;
    temp_node->fstat.st_gid = getgid();
    temp_node->fstat.st_uid = getuid();
    temp_node->fstat.st_dev = dev;
    temp_node->fstat.st_nlink = 1;

    gettimeofday(&day_tm, NULL);
    temp_node->fstat.st_atime = day_tm.tv_sec;
    temp_node->fstat.st_mtime = day_tm.tv_sec;
    temp_node->fstat.st_ctime = day_tm.tv_sec;

    return;
}

/** Create a file node
 *
 * There is no create() operation, mknod() will be called for
 * creation of all non-directory, non-symlink nodes.
 */

int nphfuse_mknod(const char *path, mode_t mode, dev_t dev)
{

    log_msg("inside mknod() for path: %s", path);
    log_msg("\n\n mode: %x :::: dev: %x", mode, dev);
    char dir_name[224];
    char file_name[128];
    uint8_t *localDBlock = NULL;
    i_node *inode_data = NULL;
    i_node *t_inode_data = NULL;
    int offset = 0;
    int i = 0;
    int check = 0;

    for (offset = 2; offset < 1000; offset++)
    {
        t_inode_data = (i_node *)data_array[offset];
        for (i = 0; i < 16; i++)
        {
            if (t_inode_data[i].dir_name[0] == '\0'){
                if (t_inode_data[i].file_name[0] == '\0')){
                        inode_data = &t_inode_data[i];
                        check = 1;
                        break;
                }
            }
        }
        if (check == 1)
            break;
    }
    log_msg("\nloop exit for mknod()\n");
    if (resolve_path(path, dir_name, file_name) != 0)
    {
        log_msg("\nresolve_path failed!\n");
        return -EINVAL;
    }
    log_msg("\n dir_name = %s\n", dir_name);

    memset(inode_data, 0, sizeof(i_node));
    strcpy(inode_data->dir_name, dir_name);
    strcpy(inode_data->file_name, file_name);
    mknod_fstat_helper(inode_data, mode, dev);

    if (npheap_getsize(npheap_fd, data_offset) != 0)
    {
        memset(inode_data, 0, sizeof(i_node));
        inode_num = inode_num - 1;
        return -ENOSPC;
    }

    localDBlock = npheap_alloc(npheap_fd, data_offset, 8192);

    if (localDBlock == NULL)
    {
        memset(inode_data, 0, sizeof(i_node));
        inode_num = inode_num - 1;
        return -ENOMEM;
    }

    memset(localDBlock, 0, 8192);
    data_array[data_offset] = localDBlock;
    inode_data->offset = data_offset++;

    log_msg("\nbefore return %d \n", inode_data->fstat.st_ino);
    return 0;
}

/* Helper function for mkdir. Assigns values to i_node struct's fstat parameters*/
void mkdir_fstat_helper(i_node *temp_node, mode_t mode)
{

    struct timeval day_tm;

    temp_node->fstat.st_ino = inode_num++;
    temp_node->fstat.st_mode = S_IFDIR | mode;
    temp_node->fstat.st_gid = getgid();
    temp_node->fstat.st_uid = getuid();
    temp_node->fstat.st_size = 4096;
    temp_node->fstat.st_nlink = 2;

    gettimeofday(&day_tm, NULL);
    temp_node->fstat.st_atime = day_tm.tv_sec;
    temp_node->fstat.st_mtime = day_tm.tv_sec;
    temp_node->fstat.st_ctime = day_tm.tv_sec;

    return;
}

/** Create a directory */
int nphfuse_mkdir(const char *path, mode_t mode)
{
    log_msg("inside mkdir for path: %s", path);
    i_node *inode_data = NULL;
    i_node *t_inode_data = NULL;
    char dir_name[224];
    char file_name[128];
    int offset = 0;
    int i = 0;
    int check = 0;
    for (offset = 2; offset < 1000; offset++)
    {
        /*        t_inode_data = (i_node *)npheap_alloc(npheap_fd, offset,
                                                npheap_getsize(npheap_fd, offset));
*/
        t_inode_data = (i_node *)data_array[offset];

        for (i = 0; i < 16; i++)
        {
            if ((t_inode_data[i].dir_name[0] == '\0') &&
                (t_inode_data[i].file_name[0] == '\0'))
            {
                log_msg("\nmkdir:: Free index:%d, offset:%d\n", i, offset);
                inode_data = &t_inode_data[i];
                log_msg("\nafter inode_data\n");
                check = 1;
                break;
            }
        }
        if (check == 1)
            break;
    }
    log_msg("\nloop exit\n");
    if (resolve_path(path, dir_name, file_name) != 0)
    {
        log_msg("\nresolve_path failed!\n");
        return -EINVAL;
    }
    log_msg("\n dir_name = %s\n", dir_name);

    memset(inode_data, 0, sizeof(i_node));
    log_msg("\n inode_data offset = %d\n", inode_data->offset);
    log_msg("\n before dir_name copy!\n");
    strcpy(inode_data->dir_name, dir_name);
    log_msg("\n before file_name copy!\n");
    strcpy(inode_data->file_name, file_name);
    log_msg("\n after file_name copy!\n");

    mkdir_fstat_helper(inode_data, mode);
    log_msg("\nbefore return %d \n", inode_data->fstat.st_ino);
    return 0;
}

/** Remove a file */
int nphfuse_unlink(const char *path)
{
    int ret =0;
    i_node *inode_data = NULL;
    log_msg("\nunlink: %s \n", path);
    inode_data = get_inode(path);
    if (inode_data == NULL)
        return -ENOENT;

    else if (CanUseInode(inode_data) != 1)
        return -EACCES;

    else if (npheap_getsize(npheap_fd, inode_data->offset) != 0)
    {
        log_msg("\nunlink: deleting offset %d \n", inode_data->offset);
        npheap_delete(npheap_fd, inode_data->offset);
    }
    log_msg("\nunlink before memset for path: %s\n", path);
    data_array[inode_data->offset] == NULL;
    memset(inode_data, 0, sizeof(inode_data));
    return ret;
}

int nphfuse_rmdir(const char *path)
{
    int ret =0;
    i_node *inode_data = NULL;
    i_node *t_inode_data = NULL;
    char dir_name[224];
    char file_name[128];
    __u64 offset = 0;
    int i = 0;

    if (strcmp(path, "/") == 0)
        inode_data = get_root_inode();

    if (resolve_path(path, dir_name, file_name) != 0)
    {
        inode_data = NULL;
    }

    for (offset = 2; offset < 1000; offset++)
    {
        t_inode_data = (i_node *)data_array[offset];

        if (t_inode_data == 0) inode_data = NULL;
   

        for (i = 0; i < 16; i++)
        {
            if ((strcmp(t_inode_data[i].dir_name, dir_name) == 0) &&
                (strcmp(t_inode_data[i].file_name, file_name) == 0))
            {
                inode_data = &t_inode_data[i];
            }
        }
    }
    int my_flag = CanUseInode(inode_data);
    if (inode_data == NULL)
    {
        log_msg("\nInside rmdir(). inode_data is NULL\n");
        return -ENOENT;
    }
    else if (my_flag != 1)
    {
        log_msg("\nInside rmdir(). Access not allowed\n");
        return -EACCES;
    }
    else
    {
        memset(inode_data, 0, 512);
        return ret;
    }
}

int nphfuse_symlink(const char *path, const char *link)
{
    log_msg("\nInside symlink() \n");
    return -1;
}

/** Rename a file */
int nphfuse_rename(const char *path, const char *newpath)
{
    struct timeval day_tm;
    i_node *inode_data = NULL;
    char dir_name[224];
    char file_name[128];
    gettimeofday(&day_tm, NULL);
    int ret =0;
    inode_data = get_inode(path);
    if (inode_data == NULL){
        log_msg("\nInside rename(). inode_data is NULL\n");
        return -ENOENT;
    }
    else if (CanUseInode(inode_data) != 1)
    {
        log_msg("\nInside rename(). Access not allowed\n");
        return -EACCES;
    }
    else if (resolve_path(newpath, dir_name, file_name) != 0)
    {
        log_msg("\nInside rename(). Unable to get directory name or file name");
        return -EINVAL;
    }
    else
    {
        memset(inode_data->dir_name, 0, 224);
        strcpy(inode_data->dir_name, dir_name);
        memset(inode_data->file_name, 0, 128);
        strcpy(inode_data->file_name, file_name);
        inode_data->fstat.st_ctime = day_tm.tv_sec;
        return ret;
    }
}

/** Create a hard link to a file */
int nphfuse_link(const char *path, const char *newpath)
{
    int ret = -1;
    log_msg("\nInside link\n");
    return ret;
}

/** Change the permission bits of a file */
int nphfuse_chmod(const char *path, mode_t mode)
{
    struct timeval day_tm;
    i_node *inode_data = NULL;
    i_node *t_inode_data = NULL;
    char dir_name[224];
    char file_name[128];
    __u64 offset = 0;
    int i = 0;
    int ret = 0;

    if (strcmp(path, "/") == 0)
        inode_data = get_root_inode();

    if (resolve_path(path, dir_name, file_name) != 0)
    {
        inode_data = NULL;
    }

    for (offset = 2; offset < 1000; offset++)
    {
        t_inode_data = (i_node *)data_array[offset];

        if (t_inode_data == 0)
        {
            inode_data = NULL;
        }

        for (i = 0; i < 16; i++)
        {
            if ((strcmp(t_inode_data[i].dir_name, dir_name) == 0) &&
                (strcmp(t_inode_data[i].file_name, file_name) == 0))
            {
                /* Entry found in inode block */
                inode_data = &t_inode_data[i];
            }
        }
    }

    if (inode_data == NULL)
    {
        log_msg("\nInside chmod(). inode_data is NULL.");
        return -ENOENT;
    }
    else if (CanUseInode(inode_data) != 1)
    {
        log_msg("\nInside chmod(). Access not allowed\n");
        return -EACCES;
    }
    else
    {
        gettimeofday(&day_tm, NULL);
        inode_data->fstat.st_ctime = day_tm.tv_sec;
        inode_data->fstat.st_mode = mode;
        return ret;
    }
}

/** Change the owner and group of a file */
int nphfuse_chown(const char *path, uid_t uid, gid_t gid)
{
    struct timeval day_tm;
    i_node *inode_data = NULL;
    i_node *t_inode_data = NULL;
    char dir_name[224];
    char file_name[128];
    __u64 offset = 0;
    int i = 0;

    if (strcmp(path, "/") == 0)
        inode_data = get_root_inode();

    if (resolve_path(path, dir_name, file_name) != 0)
    {
        inode_data = NULL;
    }

    for (offset = 2; offset < 1000; offset++)
    {
        t_inode_data = (i_node *)data_array[offset];

        if (t_inode_data == 0)
        {
            inode_data = NULL;
        }

        for (i = 0; i < 16; i++)
        {
            if ((strcmp(t_inode_data[i].dir_name, dir_name) == 0) &&
                (strcmp(t_inode_data[i].file_name, file_name) == 0))
            {
                inode_data = &t_inode_data[i];
            }
        }
    }

    if (inode_data == NULL)
    {
        log_msg("\nInside chown(). inode_data is NULL.");
        return -ENOENT;
    }
    else if (CanUseInode(inode_data) != 1)
    {
        log_msg("\nInside chown(). Access not allowed\n");
        return -EACCES;
    }
    else
    {
        inode_data->fstat.st_uid = uid;
        gettimeofday(&day_tm, NULL);
        inode_data->fstat.st_ctime = day_tm.tv_sec;
        inode_data->fstat.st_gid = gid;
        return 0;
    }
}

/** Change the size of a file */
int nphfuse_truncate(const char *path, off_t newsize)
{
    log_msg("\nInside truncate\n");
    return -ENOENT;
}

/** Change the access and/or modification times of a file */
int nphfuse_utime(const char *path, struct utimbuf *ubuf)
{
    i_node *inode_data = NULL;
    i_node *t_inode_data = NULL;
    char dir_name[224];
    char file_name[128];
    __u64 offset = 0;
    int i = 0;

    if (strcmp(path, "/") == 0)
        inode_data = get_root_inode();

    if (resolve_path(path, dir_name, file_name) != 0)
    {
        inode_data = NULL;
    }

    for (offset = 2; offset < 1000; offset++)
    {
        t_inode_data = (i_node *)data_array[offset];

        if (t_inode_data == 0) inode_data = NULL;

        for (i = 0; i < 16; i++)
        {
            if ((strcmp(t_inode_data[i].dir_name, dir_name) == 0) &&
                (strcmp(t_inode_data[i].file_name, file_name) == 0))
            {
                inode_data = &t_inode_data[i];
            }
        }
    }

    int my_flag = CanUseInode(inode_data);
    if (my_flag != 1)
    {
        log_msg("\nInside utime(). Access not allowed\n");
        return -EACCES;
    }
    if (ubuf->actime)
        inode_data->fstat.st_atime = ubuf->actime;
    if (ubuf->modtime)
        inode_data->fstat.st_mtime = ubuf->modtime;

    return 0;
}

/** File open operation
 */
int nphfuse_open(const char *path, struct fuse_file_info *fi)
{
    int ret =0;
    struct timeval day_tm;
    i_node *inode_data = get_inode(path);
    gettimeofday(&day_tm, NULL);

    if (inode_data == NULL)
        return -ENOENT;

    int my_flag = CanUseInode(inode_data);

    if (my_flag != 1)
        return -EACCES;

    fi->fh = inode_data->fstat.st_ino;
    
    inode_data->fstat.st_atime = day_tm.tv_sec;
    return ret;
}

/** Read data from an open file */
int nphfuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    log_msg("\n read: path: %s size = %d, offset = %d\n", path, size, offset);
    log_msg("\n read: buf: %s \n", buf);

    i_node *inode_data = NULL;
    uint8_t *data_block = NULL;
    size_t data_size = 0;
    struct timeval day_tm;

    size_t b_read = 0;
    size_t b_remaining = size;
    size_t read_offset = offset;
    size_t rel_offset = 0;
    size_t len =0;
    int pos = 0;
    uint8_t *next_data = NULL;
    int cur_npheap_offset = 0;

    inode_data = get_inode(path);
    if (inode_data == NULL)
        return -ENOENT;

    if (CanUseInode(inode_data) != 1)
        return -EACCES;

    data_size = npheap_getsize(npheap_fd, inode_data->offset);

    data_block = data_array[inode_data->offset];
    if (data_block == NULL)
        return -ENOENT;

    log_msg("\n read: path: %s inode->filename: %s inode->offset: %d \n", path, inode_data->file_name, inode_data->offset);
    while (b_remaining != 0)
    {
        pos = read_offset / 8192;
        cur_npheap_offset = inode_data->offset;
        while (pos != 0)
        {
            pos = pos - 1;
            cur_npheap_offset = data_next[cur_npheap_offset - 1000];
        }
        data_block = data_array[cur_npheap_offset];
        if (data_block == NULL)
            return -ENOENT;

        data_size = npheap_getsize(npheap_fd, cur_npheap_offset);
        if (data_size == 0)
            return -EINVAL;

        rel_offset = read_offset % 8192;
        if (data_size <= b_remaining + rel_offset)
        {
            len = data_size - rel_offset;
            b_remaining -= (len);
        }
        else
        {
            len = b_remaining;
            b_remaining = 0;
            log_msg("\nread: data_block:%p data:%s\n", data_block, buf);
            log_msg("\nread: path: %s b_read = %d, rel_offset = %d\n", path, b_read, rel_offset);
        }
        memcpy(buf + b_read, data_block + rel_offset,
               len);
        read_offset += (len);
        b_read += (len);
    }

    gettimeofday(&day_tm, NULL);
    inode_data->fstat.st_atime = day_tm.tv_sec;
    return b_read;
}

/** Write data to an open file */
int nphfuse_write(const char *path, const char *buf, size_t size, off_t offset,
                  struct fuse_file_info *fi)
{
    log_msg("\nwrite: path: %s size = %d, offset = %d\n", path, size, offset);
    log_msg("\nwrite: buf: %s \n", buf);
    i_node *inode_data = NULL;
    uint8_t *data_block = NULL;
    uint8_t *temp = NULL;
    size_t data_size = 0;
    int retVal = 0;
    struct timeval day_tm;
    size_t len =0;

    inode_data = get_inode(path);
    if (inode_data == NULL) return -ENOENT;
    log_msg("\nwrite: path: %s inode->filename: %s inode->offset: %d \n", path, inode_data->file_name, inode_data->offset);

    if (CanUseInode(inode_data) != 1)  return -EACCES;

    data_size = npheap_getsize(npheap_fd, inode_data->offset);
    if (data_size == 0) return 0;

    data_block = data_array[inode_data->offset];
    if (data_block == NULL)  return -ENOENT;

    size_t b_write = 0;
    size_t b_remaining = size;
    size_t write_offset = offset;
    size_t rel_offset = 0;
    uint8_t pos = 0;
    uint8_t *next_data_block = NULL;
    __u64 cur_npheap_offset = 0;

    while (b_remaining!=0){
        pos = write_offset / 8192;
        cur_npheap_offset = inode_data->offset;
        while (pos!=0){
            pos = pos - 1;
            cur_npheap_offset = data_next[cur_npheap_offset - 1000];  
        }

        data_block = data_array[cur_npheap_offset];
        if (data_block == NULL) return -ENOENT;

        data_size = npheap_getsize(npheap_fd, cur_npheap_offset);
        if (data_size == 0)
            return -EINVAL;

        rel_offset = write_offset % 8192;
        if (data_size <= b_remaining + rel_offset)
        {
            next_data_block = npheap_alloc(npheap_fd, data_offset, 8192);
            if (next_data_block == NULL)
                return -ENOMEM;

            data_array[data_offset] = next_data_block;
            data_next[cur_npheap_offset - 1000] = data_offset++;
            memset(next_data_block, 0,
                   npheap_getsize(npheap_fd, data_next[cur_npheap_offset -
                                                       1000]));
            memcpy(data_block + rel_offset, buf + b_write,
                   data_size - rel_offset);

            write_offset += (data_size - rel_offset);
            b_write += (data_size - rel_offset);
            b_remaining -= (data_size - rel_offset);
        }
        else
        {
            log_msg("\nwrite: before memcpy\n");
            memcpy(data_block + rel_offset, buf + b_write,
                   b_remaining);
            write_offset += b_remaining;
            b_write += b_remaining;
            b_remaining = 0;
            log_msg("\nwrite: data_block:%p data:%s\n", data_block, buf);
            log_msg("\nwrite: path: %s b_write = %d, rel_offset = %d\n", path, b_write, rel_offset);
        }

        retVal = b_write;
    }

    gettimeofday(&day_tm, NULL);
    inode_data->fstat.st_atime = day_tm.tv_sec;
    inode_data->fstat.st_mtime = day_tm.tv_sec;
    inode_data->fstat.st_ctime = day_tm.tv_sec;
    inode_data->fstat.st_size += retVal;

    return retVal;
}

void statfs_helper(i_node *t_inode_data, struct statvfs *statv)
{
    uint8_t inuse_block_num = 0;
    uint8_t i = 0;
    __u64 offset = 0;

    for (offset = 2; offset < 1000; offset++)
    {
        t_inode_data = (i_node *)data_array[offset];
        for (i = 0; i < 16; i++)
            if (t_inode_data[i].dir_name[0] == '\0'){
                if (t_inode_data[i].file_name[0] == '\0') 
                continue;
            }
        inuse_block_num = inuse_block_num+1;
    }

    
    statv->f_frsize = 1024;
    statv->f_blocks = 7984;
    statv->f_bsize = 1024;
    statv->f_bfree = statv->f_blocks - ((inuse_block_num - 1) / 2);
    statv->f_bavail = statv->f_bfree;
    statv->f_files = 15968;
    statv->f_ffree = statv->f_files - inuse_block_num;
    statv->f_favail = statv->f_ffree;

    return;
}

/** Get file system statistics */
int nphfuse_statfs(const char *path, struct statvfs *statv)
{
    int ret = 0;
    i_node *inode_data = NULL;
    memset(statv, 0, sizeof(struct statvfs));
    statfs_helper(inode_data, statv);
    return ret;
}

/** Possibly flush cached data  */

// this is a no-op in NPHFS.  It just logs the call and returns success
int nphfuse_flush(const char *path, struct fuse_file_info *fi)
{
    int ret =0;
    log_fi(fi);

    return ret;
}

/** Release an open file */
int nphfuse_release(const char *path, struct fuse_file_info *fi)
{
    log_msg("\nInside release() for path: %s\n", path);
    i_node *inode_data = NULL;
    i_node *t_inode_data = NULL;
    char dir_name[224];
    char file_name[128];
    __u64 offset = 0;
    int i = 0;

    if (strcmp(path, "/") == 0)
        inode_data = get_root_inode();

    if (resolve_path(path, dir_name, file_name) != 0)
    {
        inode_data = NULL;
    }

    for (offset = 2; offset < 1000; offset++)
    {
        t_inode_data = (i_node *)data_array[offset];

        if (t_inode_data == 0)
        {
            inode_data = NULL;
        }

        for (i = 0; i < 16; i++)
        {
            if ((strcmp(t_inode_data[i].dir_name, dir_name) == 0) &&
                (strcmp(t_inode_data[i].file_name, file_name) == 0))
            {
                
                inode_data = &t_inode_data[i];
            }
        }
    }

    if (inode_data == NULL)
    {
        log_msg("\nInside release(). inode_data is NULL\n");
        return -ENOENT;
    }
    else if (CanUseInode(inode_data) != 1)
    {
        log_msg("\nInside release(). Access not allowed\n");
        return -EACCES;
    }
    else
        return 0;
}

/** Synchronize file contents
 */
int nphfuse_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    int ret = -1;
    return ret;
}

#ifdef HAVE_SYS_XATTR_H
/** Set extended attributes */
int nphfuse_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
    return -61;
}

/** Get extended attributes */
int nphfuse_getxattr(const char *path, const char *name, char *value, size_t size)
{
    return -61;
}

/** List extended attributes */
int nphfuse_listxattr(const char *path, char *list, size_t size)
{
    return -61;
}

/** Remove extended attributes */
int nphfuse_removexattr(const char *path, const char *name)
{
    return -61;
}
#endif

/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this directory
 *
 * Introduced in version 2.3
 */
int nphfuse_opendir(const char *path, struct fuse_file_info *fi)
{
    log_msg("\ninside opendir for path: %s\n", path);
    i_node *inode_data = NULL;

    inode_data = get_inode(path);
    log_msg("\ninside opendir for inode filename:  %s\n", inode_data->file_name);
    if (inode_data == NULL)
    {
        return -ENOENT;
    }

    if (CanUseInode(inode_data) != 1)
    {
        return -EACCES;
    }

    return 0;
}

int nphfuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                    struct fuse_file_info *fi)
{
    struct dirent de;
    i_node *inode_data = NULL;
    log_msg("\nreaddir for path: %s\n", path);
    for (int offset = 2; offset < 1000; offset++)
    {
        //inode_data = (i_node *)npheap_alloc(npheap_fd, offset,
        //                                        npheap_getsize(npheap_fd, offset));
        inode_data = (i_node *)data_array[offset];

        //log_msg("\nreaddir before %d %d\n", block_entries, sizeof(i_node));
        //inode_data = (i_node *) data_array[offset];
        //log_msg("\nreaddir after access : %p\n", path);
        for (int i = 0; i < 16; i++)
        {
            if ((!strcmp(inode_data[i].dir_name, path)) &&
                (strcmp(inode_data[i].file_name, "/")))
            {
                //log_msg("\nreaddir before memset filename:%s\n",
                //inode_data[i].file_name);
                log_msg("\nreaddir found : %s\n", inode_data[i].file_name);
                memset(&de, 0, sizeof(de));
                strcpy(de.d_name, inode_data[i].file_name);
                if (filler(buf, de.d_name, NULL, 0) != 0)
                    return -ENOMEM;
            }
        }
        //log_msg("\nreaddir end of iteration\n");
    }
    return 0;
}

/** Release directory
 */
int nphfuse_releasedir(const char *path, struct fuse_file_info *fi)
{
    return 0;
}

/** Synchronize directory contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data
 *
 * Introduced in version 2.3
 */
// when exactly is this called?  when a user calls fsync and it
// happens to be a directory? ???
int nphfuse_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
    return 0;
}

int nphfuse_access(const char *path, int mask)
{
    char dir_name[224];
    char file_name[128];
    __u64 offset = 0;
    int i = 0;
    i_node *inode_data = NULL;
    i_node *t_inode_data = NULL;

    if (strcmp(path, "/") == 0)
        inode_data = get_root_inode();

    if (resolve_path(path, dir_name, file_name) != 0)
    {
        inode_data = NULL;
    }

    for (offset = 2; offset < 1000; offset++)
    {
        
        t_inode_data = (i_node *)data_array[offset];

        if (t_inode_data == 0)
        {
            inode_data = NULL;
        }

        for (i = 0; i < 16; i++)
        {
            if ((strcmp(t_inode_data[i].dir_name, dir_name) == 0) &&
                (strcmp(t_inode_data[i].file_name, file_name) == 0))
            {
                inode_data = &t_inode_data[i];
            }
        }
    }

    if (inode_data == NULL)
    {
        return -ENOENT;
    }
    else if (CanUseInode(inode_data) != 1)
    {
        return -EACCES;
    }
    else
    {
        return 0;
    }
}

/**
 * Change the size of an open file
 *
 * This method is called instead of the truncate() method if the
 * truncation was invoked from an ftruncate() system call.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the truncate() method will be
 * called instead.
 *
 * Introduced in version 2.5
 */
int nphfuse_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
    int  ret=-1;
    log_msg("\nInside ftruncate\n");
    return ret;
}

/**
 * Get attributes from an open file
 *
 * This method is called instead of the getattr() method if the
 * file information is available.
 *
 * Currently this is only called after the create() method if that
 * is implemented (see above).  Later it may be called for
 * invocations of fstat() too.
 *
 */
int nphfuse_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
    i_node *inode_data = NULL;
    i_node *t_inode_data = NULL;
    char dir_name[224];
    char file_name[128];
    __u64 offset = 0;
    int i = 0;

    if (strcmp(path, "/") == 0)
        inode_data = get_root_inode();

    if (resolve_path(path, dir_name, file_name) != 0)
    {
        inode_data = NULL;
    }

    for (offset = 2; offset < 1000; offset++)
    {
        t_inode_data = (i_node *)data_array[offset];

        if (t_inode_data == 0)
        {
            inode_data = NULL;
        }

        for (i = 0; i < 16; i++)
        {
            if ((strcmp(t_inode_data[i].dir_name, dir_name) == 0) &&
                (strcmp(t_inode_data[i].file_name, file_name) == 0))
            {
                inode_data = &t_inode_data[i];
            }
        }
    }

    if (inode_data == NULL)
    {
        return -ENOENT;
    }
    memcpy(statbuf, &inode_data->fstat, sizeof(struct stat));
    return 0;
}

void *nphfuse_init(struct fuse_conn_info *conn)
{
    log_msg("\nnphfuse_init()\n");
    log_conn(conn);
    log_fuse_context(fuse_get_context());
    npheap_fs_init();
    return NPHFS_DATA;
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void nphfuse_destroy(void *userdata)
{
    if (npheap_fd)
        close(npheap_fd);

    log_msg("\nnphfuse_destroy(userdata=0x%08x)\n", userdata);
}

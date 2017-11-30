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

#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

int npheap_fd = 0;
uint64_t inode_num = 2;
uint64_t data_offset = 51;

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

void split_func(char *local_pathname) {

	char* ts1 = strdup(local_pathname);
	char* ts2 = strdup(local_pathname);
	char* dir = dirname(ts1);
	char* filename = basename(ts2);
//call your funtion here in the place of the three lines below
	printf("\nDIRECTORY: %s", dir);
	printf("\nFILENAME: %s", filename);
	printf("\n-------------**-------------");
	return;
}

void path_name(char *fullpath) {

	char *pos = NULL;
	char *temp = NULL;
	char *local_pathname = NULL;
	printf("\nfullpath passed as INPUT is: %s\n", fullpath);
	for (temp = fullpath; temp <= fullpath + strlen(fullpath); temp++) {
		if(*temp == '/') {
			for (pos = temp+1 ;pos <= fullpath + strlen(fullpath); pos++) {
				if (*pos == '/' || *pos == '\0') {
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

/* Private Functions */
int GetDirFileName(const char *path, char *dir, char *file)
{
    char *string = NULL;
    char *ptr = NULL;
    char *prev = NULL;

    if (!path || !dir || !file)
    {
        return 1;
    }
    memset(dir, 0, 64);
    memset(file, 0, 32);

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
        strncat(dir, "/", 64);
        strncat(dir, prev, 32);
        prev = ptr;
    }

    if (dir[0] == '\0')
    {
        strcpy(dir, "/");
    }
    strncpy(file, prev, 32);

    printf("[%s]: dir:%s, file:%s\n", __func__, dir, file);
    free(string);
    return 0;
}
static i_node *get_root_inode(void)
{
    i_node *root_inode = NULL;
    i_node *test_inode = NULL;
    log_msg("\nget_root_inode()  called %d", npheap_getsize(npheap_fd, 2));
    root_inode = (i_node *)npheap_alloc(npheap_fd, 2,npheap_getsize(npheap_fd, 2));
    if (!root_inode)
    {
        printf("Root directory inode info not found!!\n");
        return NULL;
    }
    
    test_inode= &root_inode[0];
    log_msg("\ntest_inode links- %d, size - %d",
            test_inode->fstat.st_nlink, test_inode->fstat.st_size);
    return test_inode;
}

static i_node *get_inode(const char *path){
    i_node *inode_data = NULL;
    char dir_name[64];
    char file_name[32];
    __u64 offset = 0;
    int i = 0;

    if (strcmp(path, "/")==0) 
        return get_root_inode();

    if (GetDirFileName(path, dir_name, file_name) != 0){
        return NULL;
    }

    for (offset = 1; offset < 51; offset++)
    {
        inode_data = (i_node *)npheap_alloc(npheap_fd, offset, 8192);
        if (inode_data==0){
            printf("Fetching unsuccessful for offset: %llu, having the desired inode file:\n", offset);
            return NULL;}

        for (i = 0; i < 32; i++){
            if ((strcmp(inode_data[i].dir_name, dir_name)==0) &&
                (strcmp(inode_data[i].file_name, file_name)==0))
            {
                /* Entry found in inode block */
                return &inode_data[i];
            }
        }
    }

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
    if(npheap_getsize(npheap_fd, 1) == 0){
        log_msg("\n inside superblock allocation\n");
        block_data = npheap_alloc(npheap_fd, 1, 8192);
        if (block_data == NULL)
        {
            printf("Failed to allocate npheap memory to offset: 0");
            return;
            }
            memset(block_data, 0, npheap_getsize(npheap_fd, 1));
        }
        log_msg("\n Superblock size %d\n", npheap_getsize(npheap_fd, 1));
        log_msg("check");
        for (offset = 2; offset < 51; offset++)
        {
            log_msg("\n before alloc offset: %d-> %d\n",
                    offset, npheap_getsize(npheap_fd, offset));
            if (npheap_getsize(npheap_fd, offset) == 0)
            {
                block_data = npheap_alloc(npheap_fd, offset, 8192);
                memset(block_data, 0, npheap_getsize(npheap_fd, offset));
                log_msg("\n inode size for offset: %d-> %d\n",
                offset, npheap_getsize(npheap_fd, offset));
            }
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
    root_inode->fstat.st_size = npheap_getsize(npheap_fd, 1);
    root_inode->fstat.st_uid = getuid();
    root_inode->fstat.st_gid = getgid();
    root_inode = get_root_inode();
    return;
}


int nphfuse_getattr(const char *path, struct stat *stbuf)
{
    i_node* inode_data = NULL;
    inode_data = get_inode(path);
    if (inode_data==NULL)
        return -ENOENT;
    memcpy(stbuf, &inode_data->fstat, sizeof(struct stat));
    return 0;
    
}

/** Read the target of a symbolic link
 *
 * The buffer should be filled with a null terminated string.  The
 * buffer size argument includes the space for the terminating
 * null character.  If the linkname is too long to fit in the
 * buffer, it should be truncated.  The return value should be 0
 * for success.
 */
// Note the system readlink() will truncate and lose the terminating
// null.  So, the size passed to to the system readlink() must be one
// less than the size passed to nphfuse_readlink()
// nphfuse_readlink() code by Bernardo F Costa (thanks!)
int nphfuse_readlink(const char *path, char *link, size_t size)
{
    return -1;
}

/** Create a file node
 *
 * There is no create() operation, mknod() will be called for
 * creation of all non-directory, non-symlink nodes.
 */
int nphfuse_mknod(const char *path, mode_t mode, dev_t dev)
{
    return -ENOENT;
}

/* Helper function for mkdir. Assigns values to i_node struct's fstat parameters*/
void mkdir_fstat_helper(i_node *temp_node, mode_t mode) {

    struct timeval day_tm;

    temp_node->fstat.st_ino = inode_num++;
    temp_node->fstat.st_mode = S_IFDIR | mode;
    temp_node->fstat.st_gid = getgid();
    temp_node->fstat.st_uid = getuid();
    temp_node->fstat.st_size = 8192;
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
    char dir_name[64];
    char file_name[32];
    struct timeval day_tm;
    int offset =0;
    int i=0;
    int check = 0;
    for (offset = 2; offset < 51; offset++){
        t_inode_data = (i_node *)npheap_alloc(npheap_fd, offset,
                                                npheap_getsize(npheap_fd, offset));
        for (i = 0; i < 32; i++){
            if ((t_inode_data[i].dir_name[0] == '\0') &&
                (t_inode_data[i].file_name[0] == '\0')){
                log_msg("\nmkdir:: Free index:%d, offset:%d\n", i, offset);
                inode_data = &t_inode_data[i];
                log_msg("\nafter inode_data\n");
                check = 1;
                break;
            }
        }
        if(check==1) break;
    }
    log_msg("\nloop exit\n");
    if (GetDirFileName(path, dir_name, file_name) != 0){
        log_msg("\ngetdirfilename failed!\n");
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

/*
    inode_data->fstat.st_ino = inode_num++;
    inode_data->fstat.st_mode = S_IFDIR | mode;
    inode_data->fstat.st_nlink = 2;
    inode_data->fstat.st_size = 8192;
    inode_data->fstat.st_uid = getuid();
    inode_data->fstat.st_gid = getgid();

    gettimeofday(&day_tm, NULL);
    inode_data->fstat.st_atime = day_tm.tv_sec;
    inode_data->fstat.st_mtime = day_tm.tv_sec;
    inode_data->fstat.st_ctime = day_tm.tv_sec;
*/
    mkdir_fstat_helper(inode_data, mode);
    log_msg("\nbefore return %d \n", inode_data->fstat.st_ino);
    return 0;
}

/** Remove a file */
int nphfuse_unlink(const char *path)
{
    return -1;
}

/** Remove a directory */
int nphfuse_rmdir(const char *path)
{
    return -1;
}

/** Create a symbolic link */
// The parameters here are a little bit confusing, but do correspond
// to the symlink() system call.  The 'path' is where the link points,
// while the 'link' is the link itself.  So we need to leave the path
// unaltered, but insert the link into the mounted directory.
int nphfuse_symlink(const char *path, const char *link)
{
    return -1;
}

/** Rename a file */
// both path and newpath are fs-relative
int nphfuse_rename(const char *path, const char *newpath)
{
    return -1;
}

/** Create a hard link to a file */
int nphfuse_link(const char *path, const char *newpath)
{
    return -1;
}

/** Change the permission bits of a file */
int nphfuse_chmod(const char *path, mode_t mode)
{
        return -ENOENT;
}

/** Change the owner and group of a file */
int nphfuse_chown(const char *path, uid_t uid, gid_t gid)
{
        return -ENOENT;
}

/** Change the size of a file */
int nphfuse_truncate(const char *path, off_t newsize)
{
        return -ENOENT;
}

/** Change the access and/or modification times of a file */
int nphfuse_utime(const char *path, struct utimbuf *ubuf)
{
        return -ENOENT;
}

int CanUseInode (i_node *inode_data /*, int mode */)
{
    if (inode_data == NULL) {
        return 0;
    }

    else if ((getuid() == 0) || 
    		(getgid() == 0) ||
        	(inode_data->fstat.st_uid == getuid()) ||
        	(inode_data->fstat.st_gid == getgid())) {

        return 1;
    }

    else {
    	return 0;
	}
}



/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 * Changed in version 2.2
 */
int nphfuse_open(const char *path, struct fuse_file_info *fi)
{
	struct timeval day_tm;
 	i_node	*inode_data = get_inode(path);

 	if (inode_data == NULL) return -ENOENT;
 	
 	int my_flag =  CanUseInode(inode_data);

 	if (my_flag != 1) return -EACCES;

 	// MORE Changes Can be Made here!!! 

 	fi->fh = inode_data->fstat.st_ino;
 	gettimeofday(&day_tm, NULL);
 	inode_data->fstat.st_atime = day_tm.tv_sec;
 	npheap_lock (npheap_fd, inode_data->offset);
 	return 0;
}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * Changed in version 2.2
 */
// I don't fully understand the documentation above -- it doesn't
// match the documentation for the read() system call which says it
// can return with anything up to the amount of data requested. nor
// with the fusexmp code which returns the amount of data also
// returned by read.
int nphfuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    return -ENOENT;
}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 */
int nphfuse_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
    return -ENOENT;
}

/** Get file system statistics
 *
 * The 'f_frsize', 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
 *
 * Replaced 'struct statfs' parameter with 'struct statvfs' in
 * version 2.5
 */
int nphfuse_statfs(const char *path, struct statvfs *statv)
{
    return -1;
}

/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().  This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.  It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 *
 * Changed in version 2.2
 */

// this is a no-op in NPHFS.  It just logs the call and returns success
int nphfuse_flush(const char *path, struct fuse_file_info *fi)
{
    log_msg("\nnphfuse_flush(path=\"%s\", fi=0x%08x)\n", path, fi);
    // no need to get fpath on this one, since I work from fi->fh not the path
    log_fi(fi);
	
    return 0;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int nphfuse_release(const char *path, struct fuse_file_info *fi)
{
    return 0;
}

/** Synchronize file contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data.
 *
 * Changed in version 2.2
 */
int nphfuse_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    return -1;
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
    if (inode_data == NULL) {return -ENOENT;}

    if (CanUseInode(inode_data) != 1) {return -EACCES;}

    return 0;
}

/** Read directory
 *
 * This supersedes the old getdir() interface.  New applications
 * should use this.
 *
 * The filesystem may choose between two modes of operation:
 *
 * 1) The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * 2) The readdir implementation keeps track of the offsets of the
 * directory entries.  It uses the offset parameter and always
 * passes non-zero offset to the filler function.  When the buffer
 * is full (or an error happens) the filler function will return
 * '1'.
 *
 * Introduced in version 2.3
 */

int nphfuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi)
{
    return -ENOENT;
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
    return 0;
//    return -1;
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
    return -1;
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
        return -ENOENT;
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
        close (npheap_fd);

    log_msg("\nnphfuse_destroy(userdata=0x%08x)\n", userdata);
}



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
// code is referred from https://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/

//
#include "nphfuse.h"
#include <npheap.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *path_name(char *fullpath)
{

    char *temp = NULL, *path = NULL;
    for (temp = (fullpath + strlen(fullpath)); temp >= fullpath; temp--)
    {
        if (*temp == '/' || *temp == '\\')
        {
            break;
        }
    }

    if (temp > fullpath)
    {
        path = malloc(temp - fullpath + 1);
        strncpy(path, fullpath, (temp - fullpath));
        path[temp - fullpath] = '\0';
    }
    else if (temp == fullpath)
    {
        path = "/";
    }
    else
    {
        path = NULL;
    }
    return path;
}

char *file_name(char *fullpath)
{

    char *temp = NULL, *file = NULL;
    for (temp = (fullpath + strlen(fullpath)); temp >= fullpath; temp--)
    {
        if (*temp == '/' || *temp == '\\')
        {
            break;
        }
    }

    if (temp >= fullpath)
    {
        file = malloc(strlen(temp));
        strcpy(file, temp + 1);
    }
    else
    {
        file = malloc(strlen(fullpath) + 1);
        strcpy(file, fullpath);
    }
    return file;
}

char* full_path(const char *path){
    char *fp = malloc(PATH_MAX);
    char *file_name = NULL;
    memset(fp, 0, PATH_MAX);
    strcpy(fp, "/tmp/npheap");
    if(strcmp(path, "/")){
        strcat(fp, path);
    }
    return fp;
}

int nphfuse_getattr(const char *path, struct stat *stbuf)
{
    int ret = 0;
    fp = malloc(sizeof(PATH_MAX));
    static int first_call = 1;
    if (first_call == 1){
        system("mkdir /tmp/npheap");
        first_call= 0;
    }
    char* fp = full_path(path);
    ret = lstat(fp, stbuf);
    if(ret == -1){
        return -ENOENT;
    }
    return ret;
}

int nphfuse_readlink(const char *path, char *link, size_t size)
{
    
    int ret = 0;

    char *fp = full_path(path);
    ret = readlink(fp, link, size - 1);
    if (ret >= 0)
    {
        link[ret] = '\0';
        ret = 0;
    }
    return ret;
}

int nphfuse_mknod(const char *path, mode_t mode, dev_t dev)
{
    
    int ret = 0;

    char *fp = full_path(path);

    if (S_ISREG(mode))
    {
        ret = open(fp, O_CREAT | O_EXCL | O_WRONLY, mode);
        if (ret >= 0)
            ret = close(ret);
    }
    else if (S_ISFIFO(mode))
    {
        ret = mkfifo(fp, mode);
    }
    else
    {
        ret = mknod(fp, mode, dev);
    }

    return ret;
}

/** Create a directory */
int nphfuse_mkdir(const char *path, mode_t mode)
{
    char *fpath = full_path(path);
    return mkdir(fpath, mode);

}

/** Remove a file */
int nphfuse_unlink(const char *path)
{
    char *fp = full_path(path);
    return unlink(fp);
}

/** Remove a directory */
int nphfuse_rmdir(const char *path)
{
    char *fp = full_path(path);
    return rmdir(fp);
}

int nphfuse_symlink(const char *path, const char *link)
{
    char *fp = full_path(link);
    return symlink(path, fp);
}

/** Rename a file */
// both path and newpath are fs-relative
int nphfuse_rename(const char *path, const char *newpath)
{
    char *fp = full_path(path);
    char *nfp = full_path(newpath);
    return rename(fp, nfp);

}

/** Create a hard link to a file */
int nphfuse_link(const char *path, const char *newpath)
{
    char *fp = full_path(path);
    char *nfp = full_path(newpath);
    return link(fp, nfp);
}

/** Change the permission bits of a file */
int nphfuse_chmod(const char *path, mode_t mode)
{
    char *fp = full_path(path);
    return chmod(fp, mode);
}

/** Change the owner and group of a file */
int nphfuse_chown(const char *path, uid_t uid, gid_t gid)
{
    char *fp = full_path(path);
    return chown(fp, uid, gid);
}

/** Change the size of a file */
int nphfuse_truncate(const char *path, off_t newsize)
{
    char *fp = full_path(path);
    return truncate(fp, newsize);
}


int nphfuse_utime(const char *path, struct utimbuf *ubuf)
{
    char *fp = full_path(path);
    return utime(fp, ubuf);
}


int nphfuse_open(const char *path, struct fuse_file_info *fi)
{
    int fd;
    char *fp = full_path(path);
    fd = open(fp, fi->flags);    

    fi->fh = fd;

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
    char *fp = full_path(path);
    return pread(fi->fh, buf, size, offset);
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
    char *fp = full_path(path);
    return pwrite(fi->fh, buf, size, offset);

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

    char *fp = full_path(path);
    return statvfs(fp, statv);
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

int nphfuse_flush(const char *path, struct fuse_file_info *fi)
{
      
    return 0;
}


int nphfuse_release(const char *path, struct fuse_file_info *fi)
{
    char *fp = full_path(path);
    return close(fi->fh);
}


int nphfuse_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    return 0;
}



int nphfuse_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
    char *fp = full_path(path);
    return lsetxattr(fp, name, value, size, flags);
}


int nphfuse_getxattr(const char *path, const char *name, char *value, size_t size)
{

    char *fp = full_path(path);
    return lgetxattr(fp, name, value, size);
}

/** List extended attributes */
int nphfuse_listxattr(const char *path, char *list, size_t size)
{
    char *fp = full_path(path);
    char fp[PATH_MAX];

    fp = full_path(path);
    return llistxattr(fp, list, size);
}

/** Remove extended attributes */
int nphfuse_removexattr(const char *path, const char *name)
{
    char *fp = full_path(path);
    return lremovexattr(fp, name);
}


int nphfuse_opendir(const char *path, struct fuse_file_info *fi)
{
    
    DIR *dir = NULL;

    char *fp = full_path(path);
    dir = opendir(fp);
    fi->fh = (intptr_t)dir;
    if (!dir)
        return -EACCES; /* TODO: Change this error code */

    return 0;
}


int nphfuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                    struct fuse_file_info *fi)
{
    int retstat =0;
    DIR *dp;
    struct dirent *de;

    dp = (DIR *)(uintptr_t)fi->fh;
    de = readdir(dp);
    do {
        if (filler(buf, de->d_name, NULL, 0) != 0){
            return -ENOMEM;
        } 
    } while ((de = readdir(dp)) != NULL);
    return retstat;
}

int nphfuse_releasedir(const char *path, struct fuse_file_info *fi)
{
    closedir((DIR *)(uintptr_t)fi->fh);
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
    char *fp = full_path(path);
    return access(fp, mask);

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
    return ftruncate(fi->fh, offset);
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

    if (!strcmp(path, "/"))
        return nphfuse_getattr(path, statbuf);

    return fstat(fi->fh, statbuf);
}

void *nphfuse_init(struct fuse_conn_info *conn)
{
    log_msg("\nnphfuse_init()\n");
    log_conn(conn);
    log_fuse_context(fuse_get_context());

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
    log_msg("\nnphfuse_destroy(userdata=0x%08x)\n", userdata);
}

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
#include <stdbool.h>

extern struct nphfuse_state *nphfuse_data;
#if 0
__u64 gFreeOffset = 1;
#endif

char *GetFileName(char *str_tmp)
{
    const char s[] = "/";
    char *token;
    char *last;
    char *str = strdup(str_tmp);
    last = token = strtok(str, s);

    while ((token = strtok(NULL, s)) != NULL)
    {
        last = token;
    }

    return (last);
}

void GetFullPath(const char *path, char *fullPath)
{
    char *fileName = NULL;

    if (!path || !fullPath)
        return;

    memset(fullPath, 0, PATH_MAX);
    strcpy(fullPath, getenv("HOME"));
    strcat(fullPath, "/");
    fileName = GetFileName(nphfuse_data->device_name);
    strcat(fullPath, fileName);

    if (strcmp(path, "/"))
    {
        strncat(fullPath, path, PATH_MAX - strlen(nphfuse_data->device_name));
    }
    printf("[%s]: path:%s, fullPath:%s\n", __func__, path, fullPath);

    return;
}

bool IsRegFile(char *path)
{
    char *fileName = NULL;

    fileName = GetFileName(path);
    if ((strstr(fileName, ".swp") != NULL) ||
        (strstr(fileName, "~")))
    {
        return false;
    }

    return true;
}

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
int nphfuse_getattr(const char *path, struct stat *stbuf)
{
    char fullPath[PATH_MAX];
    char cmd[PATH_MAX];
    int retVal = 0;
    static int first = 0;

    GetFullPath(path, fullPath);
    if (!first)
    {
        memset(cmd, 0, PATH_MAX);
        strcpy(cmd, "mkdir ");
        strcat(cmd, fullPath);
        system(cmd);
        first = 1;
    }

    retVal = lstat(fullPath, stbuf);
    if (retVal == -1)
        return -ENOENT;

    return retVal;
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
    char fullPath[PATH_MAX];
    int retVal = 0;

    GetFullPath(path, fullPath);
    retVal = readlink(fullPath, link, size - 1);
    if (retVal >= 0)
    {
        link[retVal] = '\0';
        retVal = 0;
    }

    /* TODO: NPHeap code required */
    return retVal;
}

/** Create a file node
 *
 * There is no create() operation, mknod() will be called for
 * creation of all non-directory, non-symlink nodes.
 */
int nphfuse_mknod(const char *path, mode_t mode, dev_t dev)
{
    char fullPath[PATH_MAX];
    int retVal = 0;

    GetFullPath(path, fullPath);

    if (S_ISREG(mode))
    {
        retVal = open(fullPath, O_CREAT | O_EXCL | O_WRONLY, mode);
        if (retVal >= 0)
            retVal = close(retVal);
    }
    else if (S_ISFIFO(mode))
    {
        retVal = mkfifo(fullPath, mode);
    }
    else
    {
        retVal = mknod(fullPath, mode, dev);
    }

    /* TODO: NPHeap code required */
    return retVal;
}

/** Create a directory */
int nphfuse_mkdir(const char *path, mode_t mode)
{
    char fullPath[PATH_MAX];
    int retVal = 0;

    GetFullPath(path, fullPath);
    retVal = mkdir(fullPath, mode);

    return retVal;
}

/** Remove a file */
int nphfuse_unlink(const char *path)
{
    char fullPath[PATH_MAX];
    int retVal = 0;

    GetFullPath(path, fullPath);
    retVal = unlink(fullPath);

    /* TODO: NPHeap code required */
    return retVal;
}

/** Remove a directory */
int nphfuse_rmdir(const char *path)
{
    char fullPath[PATH_MAX];
    int retVal = 0;

    GetFullPath(path, fullPath);
    retVal = rmdir(fullPath);

    /* TODO: NPHeap code required */
    return retVal;
}

/** Create a symbolic link */
// The parameters here are a little bit confusing, but do correspond
// to the symlink() system call.  The 'path' is where the link points,
// while the 'link' is the link itself.  So we need to leave the path
// unaltered, but insert the link into the mounted directory.
int nphfuse_symlink(const char *path, const char *link)
{
    char linkFullPath[PATH_MAX];
    int retVal = 0;

    GetFullPath(link, linkFullPath);
    retVal = symlink(path, linkFullPath);

    /* TODO: NPHeap code required */
    return retVal;
}

/** Rename a file */
// both path and newpath are fs-relative
int nphfuse_rename(const char *path, const char *newpath)
{
    char fullPath[PATH_MAX];
    char newFullPath[PATH_MAX];
    int retVal = 0;

    GetFullPath(path, fullPath);
    GetFullPath(newpath, newFullPath);
    retVal = rename(fullPath, newFullPath);

    /* TODO: NPHeap code required */
    return retVal;
}

/** Create a hard link to a file */
int nphfuse_link(const char *path, const char *newpath)
{
    char fullPath[PATH_MAX];
    char fullNewPath[PATH_MAX];

    GetFullPath(path, fullPath);
    GetFullPath(newpath, fullNewPath);

    /* TODO: NPHeap code required */
    return (link(fullPath, fullNewPath));
}

/** Change the permission bits of a file */
int nphfuse_chmod(const char *path, mode_t mode)
{
    char fullPath[PATH_MAX];
    int retVal = 0;

    GetFullPath(path, fullPath);
    retVal = chmod(fullPath, mode);

    return retVal;
}

/** Change the owner and group of a file */
int nphfuse_chown(const char *path, uid_t uid, gid_t gid)
{
    char fullPath[PATH_MAX];
    int retVal = 0;

    GetFullPath(path, fullPath);
    retVal = chown(fullPath, uid, gid);

    return retVal;
}

/** Change the size of a file */
int nphfuse_truncate(const char *path, off_t newsize)
{
    char fullPath[PATH_MAX];
    int retVal = 0;

    GetFullPath(path, fullPath);
    retVal = truncate(fullPath, newsize);

    return retVal;
}

/** Change the access and/or modification times of a file */
int nphfuse_utime(const char *path, struct utimbuf *ubuf)
{
    char fullPath[PATH_MAX];
    int retVal = 0;

    GetFullPath(path, fullPath);
    retVal = utime(fullPath, ubuf);

    return retVal;
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
    char fullPath[PATH_MAX];
    int retVal = 0;
    int fd = 0;

    GetFullPath(path, fullPath);
    fd = open(fullPath, fi->flags);
    if (fd < 0)
    {
        return fd;
    }
    fi->fh = fd;

#if 0
    if (IsRegFile (fullPath) == false)
    {
        return 0;
    }

    /* TODO: NPHeap code required */
    /* Read file contents. If file does not contain object id
     * then allocate object id and write into file */
    tFileData fileData;
    memset (&fileData, 0, sizeof(fileData));
    if (pread (fi->fh, &fileData, sizeof(fileData), 0) <= 0)
    {
        fileData.offset = gFreeOffset++;
        printf ("[%s]: File empty..Writing offset:%llu into the file\n", __func__, fileData.offset);
        pwrite (fi->fh, &fileData, sizeof(fileData), 0);
    }

    close (fi->fh);
#endif
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
    int retVal = 0;

    retVal = pread(fi->fh, buf, size, offset);

    /* TODO: NPHeap code required */
    return retVal;
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
    char fullPath[PATH_MAX];
    int retVal = 0;

    GetFullPath(path, fullPath);
    retVal = pwrite(fi->fh, buf, size, offset);

    /* TODO: NPHeap code required */
    return retVal;
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
    char fullPath[PATH_MAX];

    GetFullPath(path, fullPath);
    return (statvfs(fullPath, statv));
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
    char fullPath[PATH_MAX];
    int retVal = 0;

    GetFullPath(path, fullPath);
    if (IsRegFile(fullPath) == false)
    {
        retVal = close(fi->fh);
    }

    /* TODO: NPHeap code required */
    return retVal;
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
    /* TODO: NPHeap code required */
    return 0;
}

#ifdef HAVE_SYS_XATTR_H
/** Set extended attributes */
int nphfuse_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
    char fullPath[PATH_MAX];

    GetFullPath(path, fullPath);
    return (lsetxattr(fullPath, name, value, size, flags));
}

/** Get extended attributes */
int nphfuse_getxattr(const char *path, const char *name, char *value, size_t size)
{
    char fullPath[PATH_MAX];

    GetFullPath(path, fullPath);
    return (lgetxattr(fullPath, name, value, size));
}

/** List extended attributes */
int nphfuse_listxattr(const char *path, char *list, size_t size)
{
    char fullPath[PATH_MAX];

    GetFullPath(path, fullPath);
    return (llistxattr(fullPath, list, size));
}

/** Remove extended attributes */
int nphfuse_removexattr(const char *path, const char *name)
{
    char fullPath[PATH_MAX];

    GetFullPath(path, fullPath);
    return (lremovexattr(fullPath, name));
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
    char fullPath[PATH_MAX];
    DIR *dir = NULL;

    GetFullPath(path, fullPath);
    dir = opendir(fullPath);
    fi->fh = (intptr_t)dir;
    if (!dir)
        return -EACCES; /* TODO: Change this error code */

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
    char fullPath[PATH_MAX];
    DIR *dir = NULL;
    struct dirent *de;

    GetFullPath(path, fullPath);
    dir = (DIR *)(uintptr_t)fi->fh;
    de = readdir(dir);
    if (!de)
        return -EACCES; /* TODO: Change this error code */

    do
    {
        if (filler(buf, de->d_name, NULL, 0) != 0)
            return -ENOMEM;

    } while ((de = readdir(dir)) != NULL);

    return 0;
}

/** Release directory
 */
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
    char fullPath[PATH_MAX];
    int retVal = 0;

    GetFullPath(path, fullPath);
    retVal = access(fullPath, mask);

    return retVal;
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
    char fullPath[PATH_MAX];
    int retVal = 0;

    GetFullPath(path, fullPath);
    retVal = ftruncate(fi->fh, offset);

    /* TODO: NPHeap code required */
    return retVal;
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
    int retVal = 0;

    if (!strcmp(path, "/"))
        return nphfuse_getattr(path, statbuf);

    return (fstat(fi->fh, statbuf));
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

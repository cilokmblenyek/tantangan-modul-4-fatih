#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>  // for DIR and struct dirent
#include <sys/stat.h> // for struct stat
#include <stdlib.h>

static const char *target_path = "/home/fatih/Documents/tantangan-modul-4";


static int myfs_getattr(const char *path, struct stat *stbuf) {
    int res;
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", target_path, path);
    res = lstat(full_path, stbuf);
    if (res == -1) return -errno;
    return 0;
}

static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    DIR *dp;
    struct dirent *de;
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", target_path, path);
    dp = opendir(full_path);
    if (dp == NULL) return -errno;
    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0)) break;
    }
    closedir(dp);
    return 0;
}

static int myfs_open(const char *path, struct fuse_file_info *fi) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", target_path, path);

    int fd = open(full_path, fi->flags);
    if (fd == -1) return -errno;

    fi->fh = fd;
    return 0;
}


static int myfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", target_path, path);
    int fd = open(full_path, O_CREAT | O_WRONLY | O_TRUNC, mode);
    if (fd == -1) {
        return -errno;
    }

    fi->fh = fd;
    return 0;
}

static int myfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd = fi->fh;
    int res;
    res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;
    return res;
}

// Function to write world.txt
void writeImageCountsToFile() {
    FILE* file = fopen("/home/fatih/Documents/tantangan-modul-4/hello.txt", "w");

    // Write the total number of images downloaded to the file
    fprintf(file, "Hello, World!\n");

    fclose(file);
}


static struct fuse_operations myfs_oper = {
    .getattr = myfs_getattr,
    .readdir = myfs_readdir,
    .open = myfs_open,
    .read = myfs_read,
    .create = myfs_create,
};

int main(int argc, char *argv[]) {
    writeImageCountsToFile();
    return fuse_main(argc, argv, &myfs_oper, NULL);
}
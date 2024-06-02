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

char* get_username() {
    struct fuse_context *context = fuse_get_context();
    if (!context) {
        return NULL; // Error: context not available
    }

    uid_t uid = context->uid;
    struct passwd *pw = getpwuid(uid);
    if (pw) {
        return strdup(pw->pw_name); // Duplicate the username string
    }

    return NULL; // Error: getpwuid failed
}


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

static int myfs_mkdir(const char *path, mode_t mode) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", target_path, path);
    char *username = get_username();
//
    printf("make for username: %s\n", username);
//
    if (strstr(full_path, username) == NULL) return -EACCES;
    int res = mkdir(full_path, mode);
    if (res == -1) return -errno;
    return 0;
}

static int myfs_rmdir(const char *path) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", target_path, path);
    char *username = get_username();
//
    printf("rm for username: %s\n", username);
//
    if (strstr(full_path, username) == NULL) return -EACCES;
    int res = rmdir(full_path);
    if (res == -1) return -errno;
    return 0;
}

static int myfs_unlink(const char *path) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", target_path, path);
    char *username = get_username();
//
    printf("unlink for username: %s\n", username);
//
    if (strstr(full_path, username) == NULL) return -EACCES;
    int res = unlink(full_path);
//
    printf("unlink in fullpath: %s\n", full_path);
//
    if (res == -1) return -errno;
    return 0;
}

static int myfs_rename(const char *from, const char *to) {
    char full_path_from[1024];
    char full_path_to[1024];
    snprintf(full_path_from, sizeof(full_path_from), "%s%s", target_path, from);
    snprintf(full_path_to, sizeof(full_path_to), "%s%s", target_path, to);
    char *username = get_username();
//
    printf("rename for username: %s\n", username);
//
    if (strstr(full_path_from, username) == NULL || strstr(full_path_to, username) == NULL) return -EACCES;
    int res = rename(full_path_from, full_path_to);
//
    printf("rename from: %s\n", full_path_from);
    printf("rename to: %s\n", full_path_to);
//
    if (res == -1) return -errno;
    return 0;
}

static int myfs_release(const char *path, struct fuse_file_info *fi) {
    return close(fi->fh);
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

static int myfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd = fi->fh;
    int res;

    char *username = get_username();
    printf("write for username: %s\n", username);
    // Write directly
    res = pwrite(fd, buf, size, offset);
    if (res == -1) res = -errno;
    return res;
}

static int myfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd = fi->fh;
    int res;

    char *username = get_username();
    printf("read for username: %s\n", username);
    res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;
    return res;
}

// Function to write world.txt
void writeImageCountsToFile() {
    FILE* file = fopen("/home/fatih/Documents/tantangan-modul-4/hello.txt", "w");
    if (file == NULL) {
        perror("Failed to open rekap.txt");
        exit(EXIT_FAILURE);
    }

    // Write the total number of images downloaded to the file
    fprintf(file, "Hello, World!\n");

    fclose(file);
}


static struct fuse_operations myfs_oper = {
    .getattr = myfs_getattr,
    .readdir = myfs_readdir,
    .open = myfs_open,
    .read = myfs_read,
    .write = myfs_write,
    .create = myfs_create,
    .release = myfs_release,
    .mkdir = myfs_mkdir,
    .rmdir = myfs_rmdir,
    .unlink = myfs_unlink,
    .rename = myfs_rename,
};

int main(int argc, char *argv[]) {
    writeImageCountsToFile();
    return fuse_main(argc, argv, &myfs_oper, NULL);
}
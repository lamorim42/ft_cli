#ifndef FS_H
# define FS_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <ctype.h>

# include <sys/stat.h>
# include <sys/types.h>
# include <dirent.h>
# include <fcntl.h>

int fs_create_dir(const char *path);
int fs_create_file(const char *path);
char *fs_read_file(const char *path);
// int fs_exists(const char *path);

#endif
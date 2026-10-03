#define _XOPEN_SOURCE 700

#include "tmpdir.h"

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *tmpdir_create(void)
{
    const char *base = getenv("TMPDIR");
    if (base == NULL || base[0] == '\0')
        base = "/tmp";

    char *path = tmpdir_path(base, "ft_test_XXXXXX");
    if (path == NULL)
        return NULL;
    if (mkdtemp(path) == NULL)
    {
        free(path);
        return NULL;
    }
    return path;
}

static int remove_entry(const char *path, const struct stat *sb, int flag, struct FTW *ftw)
{
    (void)sb;
    (void)flag;
    (void)ftw;
    return remove(path);
}

int tmpdir_remove(const char *path)
{
    return nftw(path, remove_entry, 16, FTW_DEPTH | FTW_PHYS);
}

char *tmpdir_path(const char *dir, const char *name)
{
    size_t dir_len = strlen(dir);
    size_t name_len = strlen(name);
    char *path = malloc(dir_len + 1 + name_len + 1);
    if (path == NULL)
        return NULL;

    memcpy(path, dir, dir_len);
    path[dir_len] = '/';
    memcpy(path + dir_len + 1, name, name_len + 1);
    return path;
}

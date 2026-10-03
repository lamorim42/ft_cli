#define _XOPEN_SOURCE 700

#include "capture.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static int g_fd = -1;
static int g_saved = -1;
static FILE *g_tmp = NULL;

int capture_start(int fd)
{
    fflush(stdout);
    fflush(stderr);
    g_tmp = tmpfile();
    if (g_tmp == NULL)
        return -1;
    g_saved = dup(fd);
    if (g_saved == -1 || dup2(fileno(g_tmp), fd) == -1)
    {
        fclose(g_tmp);
        g_tmp = NULL;
        return -1;
    }
    g_fd = fd;
    return 0;
}

char *capture_end(void)
{
    struct stat sb;
    char *text = NULL;

    if (g_tmp == NULL)
        return NULL;
    fflush(stdout);
    fflush(stderr);
    dup2(g_saved, g_fd);
    close(g_saved);
    if (fstat(fileno(g_tmp), &sb) == 0)
        text = malloc((size_t)sb.st_size + 1);
    if (text != NULL)
    {
        rewind(g_tmp);
        size_t len = fread(text, 1, (size_t)sb.st_size, g_tmp);
        text[len] = '\0';
    }
    fclose(g_tmp);
    g_tmp = NULL;
    g_fd = -1;
    g_saved = -1;
    return text;
}

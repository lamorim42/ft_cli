#include "output.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

int g_verbose = 0;

static void vprint(FILE *stream, const char *prefix, const char *fmt, va_list ap)
{
    int saved_errno = errno;

    if (stream == stderr)
        fflush(stdout);
    fputs(prefix, stream);
    vfprintf(stream, fmt, ap);
    fputc('\n', stream);
    errno = saved_errno;
}

void ft_err(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vprint(stderr, "ft: ", fmt, ap);
    va_end(ap);
}

void ft_warn(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vprint(stderr, "ft: aviso: ", fmt, ap);
    va_end(ap);
}

void ft_info(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vprint(stdout, "", fmt, ap);
    va_end(ap);
}

void ft_debug(const char *fmt, ...)
{
    va_list ap;

    if (!g_verbose)
        return;
    va_start(ap, fmt);
    vprint(stderr, "ft: debug: ", fmt, ap);
    va_end(ap);
}

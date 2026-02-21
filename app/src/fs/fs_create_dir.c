#include <fs.h>

int fs_create_dir(const char *path)
{
    if (mkdir(path, 0755) == -1)
    {
        perror("Error creating directory");
        return -1;
    }
    return 0;
}
#include <fs.h>

int fs_create_file(const char *path)
{
    FILE *file = fopen(path, "w");
    if (file == NULL)
    {
        perror("Error creating file");
        return -1;
    }
    fclose(file);
    return 0;
}
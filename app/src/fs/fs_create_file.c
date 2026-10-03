#include <fs.h>

// Retorna -1 com errno do sistema em caso de erro; não imprime nada.
int fs_create_file(const char *path)
{
    FILE *file = fopen(path, "w");
    if (file == NULL)
        return -1;
    fclose(file);
    return 0;
}

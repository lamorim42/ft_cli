#include <fs.h>

// Retorna -1 com errno do sistema em caso de erro; não imprime nada.
int fs_create_dir(const char *path)
{
    if (mkdir(path, 0755) == -1)
        return -1;
    return 0;
}

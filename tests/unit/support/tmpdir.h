#ifndef TMPDIR_H
#define TMPDIR_H

// Cria um diretório temporário único em $TMPDIR (ou /tmp).
// Retorna o caminho alocado com malloc, ou NULL em caso de erro.
char *tmpdir_create(void);

// Remove o diretório e todo o seu conteúdo. Retorna 0 em caso de sucesso.
int tmpdir_remove(const char *path);

// Junta dir e name com '/'. Retorna o caminho alocado com malloc.
char *tmpdir_path(const char *dir, const char *name);

#endif

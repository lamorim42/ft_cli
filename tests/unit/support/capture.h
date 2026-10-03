#ifndef CAPTURE_H
#define CAPTURE_H

// Redireciona fd (STDOUT_FILENO ou STDERR_FILENO) para um arquivo temporário.
// Retorna 0 em caso de sucesso.
int capture_start(int fd);

// Restaura fd e devolve o texto capturado, alocado com malloc (ou NULL em erro).
char *capture_end(void);

#endif

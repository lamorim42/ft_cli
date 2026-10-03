#ifndef OUTPUT_H
#define OUTPUT_H

#define FT_PRINTF(a, b) __attribute__((format(printf, a, b)))

// Liga as mensagens de ft_debug (0 por padrão).
extern int g_verbose;

// Todas acrescentam o '\n' e preservam errno.
void ft_err(const char *fmt, ...) FT_PRINTF(1, 2);  // stderr, prefixo "ft: "
void ft_warn(const char *fmt, ...) FT_PRINTF(1, 2); // stderr, prefixo "ft: aviso: "
void ft_info(const char *fmt, ...) FT_PRINTF(1, 2); // stdout, sem prefixo
void ft_debug(const char *fmt, ...)
    FT_PRINTF(1, 2); // stderr, prefixo "ft: debug: ", só se g_verbose

#endif

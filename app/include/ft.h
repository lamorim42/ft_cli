#ifndef FT_H
#define FT_H

// Códigos de saída da ft (ADR-0007).
typedef enum e_ft_status
{
    FT_OK = 0,
    FT_EUSAGE = 1,
    FT_EIO = 2,
    FT_EINTERNAL = 3,
} t_ft_status;

#endif

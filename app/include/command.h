#ifndef COMMAND_H
# define COMMAND_H

# include <stddef.h>
# include "ft_cli.h"

int cmd_new_execute(t_cli_context *ctx);

t_command g_commands[] = {
    {"new", cmd_new_execute},
    {NULL, NULL}
};

#endif
#include "ft_cli.h"
#include "command.h"
#include <string.h>
#include <stdio.h>

int dispatch_command(t_cli_context *ctx)
{
    for (int i = 0; g_commands[i].name != NULL; i++)
    {
        if (strcmp(g_commands[i].name, ctx->command) == 0)
            return g_commands[i].execute(ctx);
    }

    printf("Comando desconhecido: %s\n", ctx->command);
    return 1;
}
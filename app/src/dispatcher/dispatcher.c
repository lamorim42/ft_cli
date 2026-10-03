#include "ft_cli.h"
#include "command.h"
#include "ft.h"
#include "output.h"
#include <string.h>

int dispatch_command(t_cli_context *ctx)
{
    for (int i = 0; g_commands[i].name != NULL; i++)
    {
        if (strcmp(g_commands[i].name, ctx->command) == 0)
            return g_commands[i].execute(ctx);
    }

    ft_err("comando desconhecido: %s", ctx->command);
    return FT_EUSAGE;
}
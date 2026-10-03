#include "ft_cli.h"

int parse_args(int argc, char **argv, t_cli_context *ctx)
{
    if (argc != 4)
        return 1;

    ctx->command = argv[1];
    ctx->type = argv[2];
    ctx->project_name = argv[3];
    ctx->verbose = 0;

    return 0;
}
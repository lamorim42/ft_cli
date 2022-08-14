
#include "ft_cli.h"

void	init_cli(cli *data, int argc, char **argv)
{
	data->option = strdup(argv[1]);
	data->arg = strdup(argv[2]);
	data->name = strdup(argv[3]);
	data->dir = NULL;
	if (argc > 4)
		printf("Not implemented yet!\n");
}

void	free_cli(cli *data)
{
	free(data->option);
	free(data->arg);
	free(data->name);
	if (data->dir)
		free(data->dir);
}

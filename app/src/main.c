
#include "ft_cli.h"

int	main(int argc, char **argv)
{
	cli	data;

	if (argc != 4)
	{
		dprintf(2, "Invalid number of arguments!\n");
		return (1);
	}
	init_cli(&data, argc, argv);
	validate_option(&data);
	free_cli(&data);
	return (0);
}
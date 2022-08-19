
#include "ft_cli.h"

static void	fallback_msg(void)
{
	dprintf(STDERR_FILENO, "Usage: ft [option] [argument] [project_name]\n");
}

int	main(int argc, char **argv)
{
	t_cli	data;

	if (argc != 4)
		return (fallback_msg() , 1);
	init_cli(&data, argc, argv);
	if (is_valid_input(&data))
	{
		building_project(&data);
		write_in_files(&data);
	}
	free_cli(&data);
	return (0);
}


#include "ft_cli.h"

void	validate_option(cli *data)
{
	if (!strcmp(data->option, "new"))
		validate_arg(data);
	else
		dprintf(2, "%s: Not a valid option!\n", data->option);
}

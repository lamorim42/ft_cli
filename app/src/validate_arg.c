
#include "ft_cli.h"

void	validate_arg(cli *data)
{
	if (!strcmp(data->arg, "project"))
		validate_name(data);
	else
		dprintf(2, "%s: Not a valid argumment!\n", data->arg);
}

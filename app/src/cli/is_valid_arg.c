
#include "ft_cli.h"

int	is_valid_arg(t_cli *data)
{
	if (!strcmp(data->arg, "project"))
		return (1);
	else
		dprintf(2, "%s: Not a valid argumment!\n", data->arg);
	return (0);
}

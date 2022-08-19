
#include "ft_cli.h"

int	is_valid_name(t_cli *data)
{
	if (isalnum(data->name[0]))
		return (1);
	else
		dprintf(2, "%s: Not a valid name!\n", data->name);
	return (0);
}

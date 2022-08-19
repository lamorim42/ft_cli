
#include "ft_cli.h"

int	is_valid_option(t_cli *data)
{
	if (!strcmp(data->option, "new"))
		return (1);
	else
		dprintf(2, "%s: Not a valid option!\n", data->option);
	return (0);
}

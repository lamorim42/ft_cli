
#include "ft_cli.h"

int	is_valid_input(t_cli *data)
{
	return (is_valid_option(data)
			&& is_valid_arg(data)
			&& is_valid_name(data));
}

/**
 * is_valid_option - Checks if the provided option is valid.
 * @data: A pointer to a t_cli structure containing the option to be validated.
 *
 * This function compares the option in the t_cli structure to the string "new".
 * If the option matches "new", the function returns 1 indicating the option is valid.
 * Otherwise, it prints an error message to the standard error and returns 0.
 *
 * Return: 1 if the option is valid, 0 otherwise.
 */
#include "ft_cli.h"

int	is_valid_option(t_cli *data)
{
	if (!strcmp(data->option, "new"))
		return (1);
	else
		dprintf(2, "%s: Not a valid option!\n", data->option);
	return (0);
}

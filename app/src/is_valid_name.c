
/**
 * @file is_valid_name.c
 * @brief This file contains the implementation of the is_valid_name function.
 */

/**
 * @brief Checks if the given name in the t_cli structure is valid.
 *
 * This function checks if the first character of the name in the t_cli structure
 * is alphanumeric. If it is, the function returns 1 indicating the name is valid.
 * Otherwise, it prints an error message to the standard error and returns 0.
 *
 * @param data A pointer to a t_cli structure containing the name to be validated.
 * @return int Returns 1 if the name is valid, otherwise returns 0.
 */
#include "ft_cli.h"

int	is_valid_name(t_cli *data)
{
	if (isalnum(data->name[0]))
		return (1);
	else
		dprintf(2, "%s: Not a valid name!\n", data->name);
	return (0);
}

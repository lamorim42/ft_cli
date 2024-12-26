#include "ft_cli.h"

static void write_in_file(t_cli *data, char *file, char *text);

/**
 * @file write_in_files.c
 * @brief This file contains functions to write specific text into multiple files.
 */

/**
 * @brief Writes predefined text into multiple files.
 *
 * This function constructs file paths and corresponding text, then writes the text
 * into each file. The files include "Makefile", "README.md", an include file, and "app/src/main.c".
 *
 * @param data Pointer to the t_cli structure containing necessary data.
 */
void	write_in_files(t_cli *data)
{
	char	*include = join("app/includes/", data->include);
	char	*file[4] = {"Makefile", "README.md", include, "app/src/main.c"};
	char	*text[4] = {MAKEFILE, README, DOT_H, MAIN};
	int		i;

	i = 0;
	while (i < 4)
	{
		write_in_file(data, file[i], text[i]);
		i++;
	}
	free(include);
}

/**
 * @brief Writes text into a specified file.
 *
 * This static function constructs the full path of the file, opens it for writing,
 * and writes the provided text into it. If the text corresponds to a header file,
 * it formats the text with the name in uppercase.
 *
 * @param data Pointer to the t_cli structure containing necessary data.
 * @param file The relative path of the file to write into.
 * @param text The text to write into the file.
 */
static void	write_in_file(t_cli *data, char *file, char *text)
{
	char	*pathfile;
	int		fd;
	char	*mapi;

	pathfile = join(data->dir, file);
	mapi = NULL;
	fd = open(pathfile, O_WRONLY);
	if (!strcmp(text, DOT_H))
	{
		mapi = strmapi(data->name, &toupper);
		dprintf(fd, text, mapi, mapi);
		free(mapi);
	}
	else
		dprintf(fd, text, data->name);
	close(fd);
	free(pathfile);
}

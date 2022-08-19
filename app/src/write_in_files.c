
#include "ft_cli.h"

static void	write_in_file(t_cli *data, char *file, char *text);

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

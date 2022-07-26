
#include "ft_cli.h"

static int	make_dir(cli *data, char *cwd);
static char	*make_path_dir(char *cwd, char *name);

void	building_project(cli *data)
{
	char	cwd[500];

	getcwd(cwd, 500);
	if (!make_dir(data, cwd));
		make_structs(data);
}

static int	make_dir(cli *data, char *cwd)
{
	DIR	*dir;

	dir = opendir(data->name);
	if (dir)
	{
		dprintf(2, "%s: Directory exists\n", data->name);
		closedir(dir);
		return (-1);
	}
	else
	{
		data->dir = make_path_dir(cwd, data->name);
		mkdir(data->dir, 0775);
	}
	return (0);
}

static char	*make_path_dir(char *cwd, char *name)
{
	char	*new_dir;
	char	*temp;

	temp = join(cwd, "/");
	new_dir = join(temp, name);
	free(temp);
	return (new_dir);
}

static void	make_structs(cli *data)
{

}


#include "ft_cli.h"

static int	make_dir(char *cwd);
static char	*make_path_dir(char *cwd, char *name);
static void	make_structs(cli *data);
static void	make_file(char *path, char *file);
static void	make_dir_structs(cli *data);
static void	make_main(cli *data);
static void	make_include(cli *data);

void	building_project(cli *data)
{
	char	cwd[500];

	getcwd(cwd, 500);
	data->dir = make_path_dir(cwd, data->name);
	if (make_dir(data->name) == 0)
		make_structs(data);
}

static int	make_dir(char *pathdir)
{
	DIR	*dir;

	dir = opendir(pathdir);
	if (dir)
	{
		dprintf(2, "%s: Directory exists\n", pathdir);
		closedir(dir);
		return (-1);
	}
	else
		mkdir(pathdir, 0775);
	return (0);
}

static char	*make_path_dir(char *cwd, char *name)
{
	char	*new_dir;
	char	*temp;

	temp = join(cwd, "/");
	new_dir = join(temp, name);
	free(temp);
	temp = new_dir;
	new_dir = join(new_dir, "/");
	free(temp);
	return (new_dir);
}

static void	make_structs(cli *data)
{
	printf("Building %s\n", data->name);
	make_file(data->dir, "Makefile");
	make_file(data->dir, "README.md");
	make_file(data->dir, ".gitignore");
	make_dir_structs(data);
	make_main(data);
	make_include(data);
}

static void	make_main(cli *data)
{
	char	*pathmain;

	pathmain = join(data->dir, "app/src/");
	make_file(pathmain, "main.c");
	free(pathmain);
}

static void	make_include(cli *data)
{
	char	*pathinclude;

	data->include = join(data->name, ".h");
	pathinclude = join(data->dir, "app/includes/");
	make_file(pathinclude, data->include);
	free(pathinclude);
}

static void	make_file(char *path, char *file)
{
	char	*pathname;

	pathname = join(path, file);
	creat(pathname, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	printf("Created file %s\n", pathname);
	free(pathname);
}

static void	make_dir_structs(cli *data)
{
	char *dirs[3];

	dirs[0] = join(data->dir, "app");
	dirs[1] = join(data->dir, "app/includes");
	dirs[2] = join(data->dir, "app/src");
	for(int i = 0; i < 3; i++)
	{
		printf("Created dir %s\n", dirs[i]);
		make_dir(dirs[i]);
	}
	for(int i = 0; i < 3; i++)
		free(dirs[i]);
}

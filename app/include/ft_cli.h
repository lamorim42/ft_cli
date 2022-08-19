
#ifndef FT_CLI_H
# define FT_CLI_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <ctype.h>

# include <sys/stat.h>
# include <sys/types.h>
# include <dirent.h>
# include <fcntl.h>

# include "files_contents.h"

typedef struct s_cli
{
	char	*option;
	char	*arg;
	char	*name;
	char	*dir;
	char	*include;
} t_cli;

void	init_cli(t_cli *data, int argc, char **argv);
void	free_cli(t_cli *data);
int		is_valid_option(t_cli *data);
int		is_valid_arg(t_cli *data);
int		is_valid_name(t_cli *data);
int		is_valid_input(t_cli *data);
void	building_project(t_cli *data);

void	write_in_files(t_cli *data);

//Utils
char	*join(char *s1, char *s2);
char	*strmapi(char const *s, int (*f)(int));

#endif

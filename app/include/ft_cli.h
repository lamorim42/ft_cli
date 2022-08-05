
#ifndef FT_CLI
# define FT_CLI

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <ctype.h>

# include <sys/stat.h>
# include <sys/types.h>
# include <dirent.h>
# include <fcntl.h>

typedef struct s_cli
{
	char	*option;
	char	*arg;
	char	*name;
	char	*dir;
} cli;

void	init_cli(cli *data, int argc, char **argv);
void	free_cli(cli *data);
void	validate_option(cli *data);
void	validate_arg(cli *data);
void	validate_name(cli *data);
void	building_project(cli *data);

//Utils
char	*join(char *s1, char *s2);

#endif

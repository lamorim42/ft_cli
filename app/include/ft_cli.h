
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

# define MAKEFILE "NAME	= %s\n\
CC		= clang\n\
CFLAGS	= -Wall -Werror -Wextra -g\n\
INCLUDE	= -I ./app/include\n\
\n\
SRC = main.c\n\
\n\
OBJS = $(SRC:.c=.o)\n\
OBJ = $(addprefix ./app/obj/, $(OBJS))\n\
OBJ_DIR = ./app/obj\n\
\n\
RM = rm -rf\n\
\n\
VPATH = ./app/src\n\
\n\
$(OBJ_DIR)/%%.o: %%.c\n\
			$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@\n\
\n\
all: obj_dir $(NAME)\n\
\n\
$(NAME): $(OBJ)\n\
		$(CC) $(CFLAGS) $(OBJ) $(LIBFT) -o $(NAME)\n\
\n\
clean:\n\
	$(RM) $(OBJ)\n\
\n\
fclean:\n\
	$(RM) $(OBJ)\n\
	$(RM) $(OBJ_DIR)\n\
	$(RM) $(NAME)\n\
\n\
re: fclean all\n\
\n\
obj_dir:\n\
	@if [ ! -d \"$(OBJ_DIR)\" ]; then\n\
		mkdir $(OBJ_DIR);\n\
	else\n\
		echo \"make: Nothing to be done for 'all'.\";\n\
	fi\n\
\n\
.PHONY= all clean fclean re $(NAME)"

# define README "# Hello %s\n"

# define DOT_H "\n#ifndef %s_H\n\
# define %s_H\n\
\n\
#endif\n"

# define MAIN "\n#include \"%s.h\"\n\
\n\
int	main(void)\n\
{\n\
\n\
}\n"
typedef struct s_cli
{
	char	*option;
	char	*arg;
	char	*name;
	char	*dir;
	char	*include;
} cli;

void	init_cli(cli *data, int argc, char **argv);
void	free_cli(cli *data);
void	validate_option(cli *data);
void	validate_arg(cli *data);
void	validate_name(cli *data);
void	building_project(cli *data);

void	write_in_files(cli *data);

//Utils
char	*join(char *s1, char *s2);
char	*strmapi(char const *s, int (*f)(int));

#endif

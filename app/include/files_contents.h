
#ifndef FILES_CONTENTS_H
# define FILES_CONTENTS_H

# define MAKEFILE "NAME	= %s\n\
CC		= clang\n\
CFLAGS	= -Wall -Werror -Wextra -g\n\
INCLUDE	= -I ./app/includes\n\
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
	@if [ ! -d \"$(OBJ_DIR)\" ]; then\\\n\
		mkdir $(OBJ_DIR);\\\n\
	else\\\n\
		echo \"make: Nothing to be done for 'all'.\";\\\n\
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
\treturn (0);\n\
}\n"

# define FALLBACK "Usage:\tft [option] [argument] [project_name]\n\
      \tTry 'ft --help' for more information.\n"

# define HELP "Usage:\tft [option] [argument] [project_name]\n\
\nOptions:\n\
\t- new: Creat a new argument\n\
\n\
Argumens:\n\
\t- projec: Use to creat new project\n\
\n\
Exemple: ft new project my_project\n"

#endif
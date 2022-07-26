
NAME	= ft
CC		= clang
CFLAGS	= -Wall -Werror -Wextra -g
INCLUDE	= -I ./app/include

SRC = main.c
SRC += init_cli.c
SRC += validate_option.c
SRC += validate_arg.c
SRC += validate_name.c
SRC += building_project.c
SRC += cli_utils.c

OBJS = $(SRC:.c=.o)
OBJ = $(addprefix ./app/obj/, $(OBJS))
OBJ_DIR = ./app/obj

RM = rm -rf

VPATH = ./app/src

$(OBJ_DIR)/%.o: %.c
			$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

all: obj_dir $(NAME)

$(NAME): $(OBJ)
		$(CC) $(CFLAGS) $(OBJ) $(LIBFT) -o $(NAME)

clean:
	$(RM) $(OBJ)

fclean:
	$(RM) $(OBJ)
	$(RM) $(OBJ_DIR)
	$(RM) $(NAME)

re: fclean all

obj_dir:
	@mkdir -p $(OBJ_DIR)

.PHONY= all clean fclean re $(NAME)

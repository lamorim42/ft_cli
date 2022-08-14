
NAME	= ft
CC		= clang
CFLAGS	= -Wall -Werror -Wextra -g
INCLUDE	= -I ./app/include

SRC = main.c
SRC += init_and_free_cli.c
SRC += validate_option.c
SRC += validate_arg.c
SRC += validate_name.c
SRC += building_project.c
SRC += cli_utils.c
SRC += write_in_files.c

OBJS = $(SRC:.c=.o)
OBJ = $(addprefix ./app/obj/, $(OBJS))
OBJ_DIR = ./app/obj

RM = rm -rf

VPATH = ./app/src

LOCAL_INSTALL = /usr/local/bin

$(OBJ_DIR)/%.o: %.c
			$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

all: obj_dir $(NAME)

$(NAME): $(OBJ)
		$(CC) $(CFLAGS) $(OBJ) $(LIBFT) -o $(NAME)

clean:
	$(RM) $(OBJ)
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

install: $(NAME)
	cp $(NAME) $(LOCAL_INSTALL)

re: fclean all

obj_dir:
	@if [ ! -d "$(OBJ_DIR)" ]; then\
		mkdir $(OBJ_DIR);\
	else\
		echo "make: Nothing to be done for 'all'.";\
	fi

.PHONY= all clean fclean re $(NAME)


NAME	= ft
CC		= clang
CFLAGS	= -Wall -Werror -Wextra -g
INCLUDE	= -I app/include

# Fontes organizados por subdiretórios
SRC = \
	app/src/main.c \
	app/src/cli/init_and_free_cli.c \
	app/src/cli/is_valid_option.c \
	app/src/cli/is_valid_arg.c \
	app/src/cli/is_valid_name.c \
	app/src/cli/input_validation.c \
	app/src/core/building_project.c \
	app/src/core/write_in_files.c \
	app/src/utils/cli_utils.c \
	app/src/commands/cmd_new.c \
	app/src/dispatcher/dispatcher.c \
	app/src/parser/parser.c \
# 	app/src/fs/fs_create_dir.c \
# 	app/src/fs/fs_create_file.c

OBJ_DIR = app/obj
OBJS = $(SRC:app/src/%.c=$(OBJ_DIR)/%.o)

RM = rm -rf

VPATH = ./app/src

LOCAL_INSTALL = /usr/local/bin


# Garante que o subdiretório de destino existe antes de compilar
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

all: obj_dir $(NAME)

$(NAME): $(OBJS)
		$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

clean:
	$(RM) $(OBJS)
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

install: $(NAME)
	cp $(NAME) $(LOCAL_INSTALL)

re: fclean all

obj_dir:
	@mkdir -p $(OBJ_DIR)

.PHONY= all clean fclean re install $(NAME)

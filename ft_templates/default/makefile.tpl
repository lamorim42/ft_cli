NAME	= {{PROJECT_NAME}}
CC		= clang
CFLAGS	= -Wall -Werror -Wextra -g
INCLUDE	= -I ./app/includes

SRC = main.c

OBJS = $(notdir $(SRC:.c=.o))
OBJ = $(addprefix ./app/obj/, $(OBJS))
OBJ_DIR = ./app/obj

RM = rm -rf

VPATH = ./app/src

$(OBJ_DIR)/%.o: app/src/%.c
			$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

all: obj_dir $(NAME)

$(NAME): $(OBJ)
		$(CC) $(CFLAGS) $(OBJ) $(LIBFT) -o app/bin/$(NAME)
		cp app/bin/$(NAME) ./$(NAME)

clean:
	$(RM) $(OBJ)

fclean:
	$(RM) $(OBJ)
	$(RM) $(OBJ_DIR)
	$(RM) app/bin/$(NAME)
	$(RM) $(NAME)

re: fclean all

obj_dir:
	@if [ ! -d "$(OBJ_DIR)" ]; then\
		mkdir $(OBJ_DIR);\
	else\
		echo "make: Nothing to be done for 'all'.";\
	fi

.PHONY= all clean fclean re $(NAME)
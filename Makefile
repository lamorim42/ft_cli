
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
	app/src/fs/fs_create_dir.c \
	app/src/fs/fs_create_file.c \
	app/src/fs/fs_read_file.c \
	app/src/output/output.c

OBJ_DIR = app/obj
OBJS = $(SRC:app/src/%.c=$(OBJ_DIR)/%.o)
APP_OBJS = $(filter-out $(OBJ_DIR)/main.o,$(OBJS))

# Testes unitários (Unity em tests/vendor/unity)
TEST_DIR	= tests
UNITY_DIR	= $(TEST_DIR)/vendor/unity
TEST_SRC	= $(wildcard $(TEST_DIR)/unit/test_*.c)
TEST_SUP	= $(wildcard $(TEST_DIR)/unit/support/*.c)
TEST_BINS	= $(TEST_SRC:$(TEST_DIR)/unit/%.c=$(TEST_DIR)/bin/%)
TEST_FLAGS	= $(INCLUDE) -I $(UNITY_DIR) -I $(TEST_DIR)/unit/support

# Testes funcionais (shell POSIX em tests/functional)
FUNC_RUNNER	= $(TEST_DIR)/run.sh

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
	$(RM) $(TEST_DIR)/bin $(TEST_DIR)/obj

install: $(NAME)
	cp $(NAME) $(LOCAL_INSTALL)
	cp ./ft_templates/ $(LOCAL_INSTALL) -r

re: fclean all

$(TEST_DIR)/bin/%: $(TEST_DIR)/unit/%.c $(TEST_SUP) $(UNITY_DIR)/unity.c $(APP_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(TEST_FLAGS) $^ -o $@

test-unit: $(TEST_BINS)
	@fail=0; for t in $(TEST_BINS); do ./$$t || fail=1; done; exit $$fail

test-functional: $(NAME)
	@sh $(FUNC_RUNNER) ./$(NAME)

# Roda as duas suítes e só falha no fim, para uma não esconder a outra.
test:
	@fail=0; \
	$(MAKE) --no-print-directory test-unit || fail=1; \
	$(MAKE) --no-print-directory test-functional || fail=1; \
	exit $$fail

.PHONY: test test-unit test-functional

obj_dir:
	@mkdir -p $(OBJ_DIR)

.PHONY= all clean fclean re install $(NAME)

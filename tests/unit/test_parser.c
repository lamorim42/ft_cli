#include "ft_cli.h"
#include "unity.h"

static t_cli_context ctx;

void setUp(void)
{
    memset(&ctx, 0, sizeof(ctx));
    ctx.verbose = 42;
}

void tearDown(void)
{
}

static void test_four_args_fill_context(void)
{
    char *argv[] = {"ft", "new", "project", "minishell", NULL};

    TEST_ASSERT_EQUAL_INT(0, parse_args(4, argv, &ctx));
    TEST_ASSERT_EQUAL_STRING("new", ctx.command);
    TEST_ASSERT_EQUAL_STRING("project", ctx.type);
    TEST_ASSERT_EQUAL_STRING("minishell", ctx.project_name);
    TEST_ASSERT_EQUAL_INT(0, ctx.verbose);
}

// Comportamento atual: só aceita exatamente 4 argumentos (o passo 3b muda isso).
static void test_other_argc_returns_error(void)
{
    char *argv[] = {"ft", "new", "project", "minishell", "extra", NULL};
    int argcs[] = {1, 2, 3, 5};

    for (size_t i = 0; i < sizeof(argcs) / sizeof(argcs[0]); i++)
        TEST_ASSERT_EQUAL_INT_MESSAGE(1, parse_args(argcs[i], argv, &ctx), "argc rejeitado");
}

static void test_help_flag_is_rejected(void)
{
    char *argv[] = {"ft", "--help", NULL};

    TEST_ASSERT_EQUAL_INT(1, parse_args(2, argv, &ctx));
}

static void test_short_help_flag_is_rejected(void)
{
    char *argv[] = {"ft", "-h", NULL};

    TEST_ASSERT_EQUAL_INT(1, parse_args(2, argv, &ctx));
}

static void test_fields_point_into_argv(void)
{
    char *argv[] = {"ft", "new", "project", "minishell", NULL};

    TEST_ASSERT_EQUAL_INT(0, parse_args(4, argv, &ctx));
    TEST_ASSERT_EQUAL_PTR(argv[1], ctx.command);
    TEST_ASSERT_EQUAL_PTR(argv[2], ctx.type);
    TEST_ASSERT_EQUAL_PTR(argv[3], ctx.project_name);
}

// Comportamento atual: flags não são reconhecidas (o passo 3 traz o parser novo).
static void test_flag_in_name_position_becomes_name(void)
{
    char *argv[] = {"ft", "new", "project", "--force", NULL};

    TEST_ASSERT_EQUAL_INT(0, parse_args(4, argv, &ctx));
    TEST_ASSERT_EQUAL_STRING("--force", ctx.project_name);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_four_args_fill_context);
    RUN_TEST(test_other_argc_returns_error);
    RUN_TEST(test_help_flag_is_rejected);
    RUN_TEST(test_short_help_flag_is_rejected);
    RUN_TEST(test_fields_point_into_argv);
    RUN_TEST(test_flag_in_name_position_becomes_name);
    return UNITY_END();
}

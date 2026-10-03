// Não inclui command.h: ele define g_commands, que já vem de dispatcher.o.
#include "ft_cli.h"
#include "unity.h"

void setUp(void)
{
}

void tearDown(void)
{
}

static void test_unknown_command_returns_error(void)
{
    t_cli_context ctx = {.command = "foo", .type = "project", .project_name = "x", .verbose = 0};

    TEST_ASSERT_EQUAL_INT(1, dispatch_command(&ctx));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_unknown_command_returns_error);
    return UNITY_END();
}

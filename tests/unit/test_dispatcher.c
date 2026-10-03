// Não inclui command.h: ele define g_commands, que já vem de dispatcher.o.
#include "capture.h"
#include "ft.h"
#include "ft_cli.h"
#include "unity.h"

void setUp(void)
{
}

void tearDown(void)
{
}

// A5: erro em stderr com prefixo, nada em stdout, código de uso.
static void assert_unknown(const char *command, const char *expected_err)
{
    t_cli_context ctx = {.command = (char *)command, .type = "project", .project_name = "x"};

    TEST_ASSERT_EQUAL_INT(0, capture_start(STDOUT_FILENO));
    TEST_ASSERT_EQUAL_INT(FT_EUSAGE, dispatch_command(&ctx));
    char *out = capture_end();
    TEST_ASSERT_EQUAL_STRING("", out);
    free(out);

    TEST_ASSERT_EQUAL_INT(0, capture_start(STDERR_FILENO));
    TEST_ASSERT_EQUAL_INT(FT_EUSAGE, dispatch_command(&ctx));
    char *err = capture_end();
    TEST_ASSERT_EQUAL_STRING(expected_err, err);
    free(err);
}

static void test_unknown_command_returns_error(void)
{
    assert_unknown("foo", "ft: comando desconhecido: foo\n");
}

static void test_command_name_is_case_sensitive(void)
{
    assert_unknown("NEW", "ft: comando desconhecido: NEW\n");
}

static void test_empty_command_returns_error(void)
{
    assert_unknown("", "ft: comando desconhecido: \n");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_unknown_command_returns_error);
    RUN_TEST(test_command_name_is_case_sensitive);
    RUN_TEST(test_empty_command_returns_error);
    return UNITY_END();
}

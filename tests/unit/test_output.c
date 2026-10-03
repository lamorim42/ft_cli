#include "capture.h"
#include "output.h"
#include "unity.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void setUp(void)
{
    g_verbose = 0;
}

void tearDown(void)
{
    g_verbose = 0;
}

static void assert_captured(int fd, const char *expected)
{
    char *text = capture_end();

    TEST_ASSERT_NOT_NULL(text);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, text, fd == STDOUT_FILENO ? "stdout" : "stderr");
    free(text);
}

static void test_capture_restores_descriptor(void)
{
    TEST_ASSERT_EQUAL_INT(0, capture_start(STDOUT_FILENO));
    printf("capturado\n");
    assert_captured(STDOUT_FILENO, "capturado\n");
    printf("depois da captura (deve aparecer na saida do teste)\n");
}

static void test_err_writes_prefixed_line_to_stderr(void)
{
    TEST_ASSERT_EQUAL_INT(0, capture_start(STDERR_FILENO));
    ft_err("x %d", 42);
    assert_captured(STDERR_FILENO, "ft: x 42\n");
}

static void test_err_writes_nothing_to_stdout(void)
{
    TEST_ASSERT_EQUAL_INT(0, capture_start(STDOUT_FILENO));
    ft_err("x %d", 42);
    assert_captured(STDOUT_FILENO, "");
}

static void test_warn_uses_warning_prefix(void)
{
    TEST_ASSERT_EQUAL_INT(0, capture_start(STDERR_FILENO));
    ft_warn("chave desconhecida: %s", "FOO");
    assert_captured(STDERR_FILENO, "ft: aviso: chave desconhecida: FOO\n");
}

static void test_info_writes_to_stdout_without_prefix(void)
{
    TEST_ASSERT_EQUAL_INT(0, capture_start(STDOUT_FILENO));
    ft_info("criado  %s", "foo/Makefile");
    assert_captured(STDOUT_FILENO, "criado  foo/Makefile\n");
}

static void test_info_writes_nothing_to_stderr(void)
{
    TEST_ASSERT_EQUAL_INT(0, capture_start(STDERR_FILENO));
    ft_info("criado  %s", "foo/Makefile");
    assert_captured(STDERR_FILENO, "");
}

static void test_debug_is_silent_without_verbose(void)
{
    TEST_ASSERT_EQUAL_INT(0, capture_start(STDERR_FILENO));
    ft_debug("template %s", "default");
    assert_captured(STDERR_FILENO, "");
}

static void test_debug_writes_with_verbose(void)
{
    g_verbose = 1;
    TEST_ASSERT_EQUAL_INT(0, capture_start(STDERR_FILENO));
    ft_debug("template %s", "default");
    assert_captured(STDERR_FILENO, "ft: debug: template default\n");
}

static void test_err_preserves_errno(void)
{
    TEST_ASSERT_EQUAL_INT(0, capture_start(STDERR_FILENO));
    errno = ENOENT;
    ft_err("x");
    int after = errno;
    assert_captured(STDERR_FILENO, "ft: x\n");
    TEST_ASSERT_EQUAL_INT(ENOENT, after);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_capture_restores_descriptor);
    RUN_TEST(test_err_writes_prefixed_line_to_stderr);
    RUN_TEST(test_err_writes_nothing_to_stdout);
    RUN_TEST(test_warn_uses_warning_prefix);
    RUN_TEST(test_info_writes_to_stdout_without_prefix);
    RUN_TEST(test_info_writes_nothing_to_stderr);
    RUN_TEST(test_debug_is_silent_without_verbose);
    RUN_TEST(test_debug_writes_with_verbose);
    RUN_TEST(test_err_preserves_errno);
    return UNITY_END();
}

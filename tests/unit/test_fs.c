#include "capture.h"
#include "fs.h"
#include "tmpdir.h"
#include "unity.h"

#include <errno.h>

static char *dir;

void setUp(void)
{
    dir = tmpdir_create();
    TEST_ASSERT_NOT_NULL(dir);
}

void tearDown(void)
{
    tmpdir_remove(dir);
    free(dir);
    dir = NULL;
}

static int is_dir(const char *path)
{
    struct stat sb;
    return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);
}

static void write_file(const char *path, const char *content)
{
    FILE *f = fopen(path, "w");
    TEST_ASSERT_NOT_NULL(f);
    fputs(content, f);
    fclose(f);
}

static long file_size(const char *path)
{
    struct stat sb;
    TEST_ASSERT_EQUAL_INT(0, stat(path, &sb));
    return (long)sb.st_size;
}

static void test_create_dir_creates_directory(void)
{
    char *path = tmpdir_path(dir, "novo");

    TEST_ASSERT_EQUAL_INT(0, fs_create_dir(path));
    TEST_ASSERT_TRUE(is_dir(path));
    free(path);
}

static void test_create_dir_fails_when_exists(void)
{
    char *path = tmpdir_path(dir, "novo");

    TEST_ASSERT_EQUAL_INT(0, fs_create_dir(path));
    TEST_ASSERT_EQUAL_INT(-1, fs_create_dir(path));
    free(path);
}

static void test_create_file_creates_empty_file(void)
{
    char *path = tmpdir_path(dir, "vazio.txt");

    TEST_ASSERT_EQUAL_INT(0, fs_create_file(path));
    TEST_ASSERT_EQUAL_INT(0, file_size(path));
    free(path);
}

// Comportamento atual: sobrescreve sem avisar (o passo 8 traz a proteção).
static void test_create_file_truncates_existing(void)
{
    char *path = tmpdir_path(dir, "existente.txt");
    write_file(path, "trabalho do usuario");

    TEST_ASSERT_EQUAL_INT(0, fs_create_file(path));
    TEST_ASSERT_EQUAL_INT(0, file_size(path));
    free(path);
}

static void test_read_file_returns_exact_content(void)
{
    char *path = tmpdir_path(dir, "conteudo.txt");
    write_file(path, "linha 1\nlinha 2\n{{PROJECT_NAME}}\n");

    char *content = fs_read_file(path);
    TEST_ASSERT_EQUAL_STRING("linha 1\nlinha 2\n{{PROJECT_NAME}}\n", content);
    free(content);
    free(path);
}

static void test_read_file_empty_returns_empty_string(void)
{
    char *path = tmpdir_path(dir, "vazio.txt");
    write_file(path, "");

    char *content = fs_read_file(path);
    TEST_ASSERT_EQUAL_STRING("", content);
    free(content);
    free(path);
}

static void test_read_file_missing_returns_null(void)
{
    char *path = tmpdir_path(dir, "nao_existe.txt");

    TEST_ASSERT_NULL(fs_read_file(path));
    free(path);
}

// Comportamento atual: sem mkdir -p (o passo 5 traz o fs_mkdir_p).
static void test_create_dir_fails_without_parent(void)
{
    char *path = tmpdir_path(dir, "pai/filho");

    TEST_ASSERT_EQUAL_INT(-1, fs_create_dir(path));
    TEST_ASSERT_EQUAL_INT(-1, access(path, F_OK));
    free(path);
}

static void test_create_dir_fails_over_regular_file(void)
{
    char *path = tmpdir_path(dir, "arquivo");
    write_file(path, "x");

    TEST_ASSERT_EQUAL_INT(-1, fs_create_dir(path));
    TEST_ASSERT_FALSE(is_dir(path));
    free(path);
}

static void test_create_dir_uses_mode_0755(void)
{
    char *path = tmpdir_path(dir, "modo");
    mode_t old = umask(022);
    struct stat sb;

    TEST_ASSERT_EQUAL_INT(0, fs_create_dir(path));
    umask(old);
    TEST_ASSERT_EQUAL_INT(0, stat(path, &sb));
    TEST_ASSERT_EQUAL_UINT(0755, sb.st_mode & 0777);
    free(path);
}

static void test_create_file_fails_without_parent(void)
{
    char *path = tmpdir_path(dir, "pai/arquivo.txt");

    TEST_ASSERT_EQUAL_INT(-1, fs_create_file(path));
    TEST_ASSERT_EQUAL_INT(-1, access(path, F_OK));
    free(path);
}

static void test_read_file_large_content(void)
{
    char *path = tmpdir_path(dir, "grande.txt");
    size_t size = 64 * 1024;
    char *expected = malloc(size + 1);
    TEST_ASSERT_NOT_NULL(expected);
    for (size_t i = 0; i < size; i++)
        expected[i] = (char)('a' + i % 26);
    expected[size] = '\0';
    write_file(path, expected);

    char *content = fs_read_file(path);
    TEST_ASSERT_NOT_NULL(content);
    TEST_ASSERT_EQUAL_size_t(size, strlen(content));
    TEST_ASSERT_EQUAL_MEMORY(expected, content, size + 1);
    free(content);
    free(expected);
    free(path);
}

static void test_read_file_without_trailing_newline(void)
{
    char *path = tmpdir_path(dir, "sem_newline.txt");
    write_file(path, "# Hello {{PROJECT_NAME}}");

    char *content = fs_read_file(path);
    TEST_ASSERT_EQUAL_STRING("# Hello {{PROJECT_NAME}}", content);
    free(content);
    free(path);
}

static void test_create_dir_failure_is_silent(void)
{
    char *path = tmpdir_path(dir, "pai/filho");

    TEST_ASSERT_EQUAL_INT(0, capture_start(STDERR_FILENO));
    int ret = fs_create_dir(path);
    char *err = capture_end();
    TEST_ASSERT_EQUAL_INT(-1, ret);
    TEST_ASSERT_EQUAL_STRING("", err);
    free(err);
    free(path);
}

static void test_create_dir_existing_sets_eexist(void)
{
    char *path = tmpdir_path(dir, "novo");

    TEST_ASSERT_EQUAL_INT(0, fs_create_dir(path));
    errno = 0;
    TEST_ASSERT_EQUAL_INT(-1, fs_create_dir(path));
    TEST_ASSERT_EQUAL_INT(EEXIST, errno);
    free(path);
}

static void test_create_file_failure_is_silent(void)
{
    char *path = tmpdir_path(dir, "pai/arquivo.txt");

    TEST_ASSERT_EQUAL_INT(0, capture_start(STDERR_FILENO));
    int ret = fs_create_file(path);
    char *err = capture_end();
    TEST_ASSERT_EQUAL_INT(-1, ret);
    TEST_ASSERT_EQUAL_STRING("", err);
    free(err);
    free(path);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_create_dir_creates_directory);
    RUN_TEST(test_create_dir_fails_when_exists);
    RUN_TEST(test_create_file_creates_empty_file);
    RUN_TEST(test_create_file_truncates_existing);
    RUN_TEST(test_read_file_returns_exact_content);
    RUN_TEST(test_read_file_empty_returns_empty_string);
    RUN_TEST(test_read_file_missing_returns_null);
    RUN_TEST(test_create_dir_fails_without_parent);
    RUN_TEST(test_create_dir_fails_over_regular_file);
    RUN_TEST(test_create_dir_uses_mode_0755);
    RUN_TEST(test_create_file_fails_without_parent);
    RUN_TEST(test_read_file_large_content);
    RUN_TEST(test_read_file_without_trailing_newline);
    RUN_TEST(test_create_dir_failure_is_silent);
    RUN_TEST(test_create_dir_existing_sets_eexist);
    RUN_TEST(test_create_file_failure_is_silent);
    return UNITY_END();
}

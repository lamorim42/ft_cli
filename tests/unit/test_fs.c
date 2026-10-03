#include "fs.h"
#include "tmpdir.h"
#include "unity.h"

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
    return UNITY_END();
}

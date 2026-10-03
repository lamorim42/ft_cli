# Helpers dos testes funcionais. Carregado por cada tests/functional/test_*.sh.
#
# Variáveis exportadas pelo tests/run.sh:
#   FT   caminho absoluto do binário ft testado
#   CAP  diretório para capturas, fora do diretório corrente do teste
#
# Protocolo: o teste sai com 0 (passou), 77 (pulado) ou outro código (falhou).
# A primeira asserção que falha encerra o teste.

fail()
{
    printf 'FALHA: %s\n' "$*" >&2
    exit 1
}

skip()
{
    printf 'pulado: %s\n' "$*"
    exit 77
}

show_output()
{
    printf -- '--- stdout ---\n' >&2
    cat "$CAP/out" >&2
    printf -- '--- stderr ---\n' >&2
    cat "$CAP/err" >&2
}

# Roda o ft com stdout em $CAP/out e stderr em $CAP/err; o código fica em FT_STATUS.
ft_run()
{
    "$FT" "$@" >"$CAP/out" 2>"$CAP/err"
    FT_STATUS=$?
}

assert_status()
{
    if [ "$FT_STATUS" -ne "$1" ]; then
        show_output
        fail "código de saída esperado $1, obtido $FT_STATUS"
    fi
}

# Compara um arquivo com o stdin (texto esperado), mostrando o diff.
compare_with_stdin()
{
    cat >"$CAP/expected"
    if ! diff -u "$CAP/expected" "$1" >"$CAP/diff"; then
        cat "$CAP/diff" >&2
        fail "$2"
    fi
}

assert_stdout()
{
    compare_with_stdin "$CAP/out" "stdout diferente do esperado"
}

assert_stderr()
{
    compare_with_stdin "$CAP/err" "stderr diferente do esperado"
}

assert_stdout_empty()
{
    if [ -s "$CAP/out" ]; then
        show_output
        fail "stdout deveria estar vazio"
    fi
}

assert_stderr_empty()
{
    if [ -s "$CAP/err" ]; then
        show_output
        fail "stderr deveria estar vazio"
    fi
}

# Compara a árvore de $1 (find | sort) com o stdin.
assert_tree()
{
    [ -d "$1" ] || fail "diretório não existe: $1"
    find "$1" | LC_ALL=C sort >"$CAP/tree"
    compare_with_stdin "$CAP/tree" "árvore de $1 diferente da esperada"
}

assert_file_eq()
{
    [ -f "$1" ] || fail "arquivo não existe: $1"
    compare_with_stdin "$1" "conteúdo de $1 diferente do esperado"
}

assert_exists()
{
    [ -e "$1" ] || fail "deveria existir: $1"
}

assert_not_exists()
{
    if [ -e "$1" ] || [ -L "$1" ]; then
        fail "não deveria existir: $1"
    fi
}

# Falha se o arquivo $1 não contiver a linha exata $2.
assert_file_has_line()
{
    [ -f "$1" ] || fail "arquivo não existe: $1"
    grep -qxF -- "$2" "$1" || fail "linha não encontrada em $1: $2"
}

# Os testes que geram projeto dependem de /proc/self/exe (D6, passo 9 do TDD).
require_linux()
{
    [ "$(uname -s)" = Linux ] || skip "a geração usa /proc/self/exe, só no Linux (D6)"
}

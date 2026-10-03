#!/bin/sh
# Runner dos testes funcionais: sh tests/run.sh <caminho do ft>
#
# Cada tests/functional/test_*.sh roda num diretório temporário próprio.
# Códigos de saída de um teste: 0 passou, 77 pulado, outro falhou.
# FT_TEST_KEEP=1 mantém os diretórios temporários para depuração.
set -u

usage()
{
    printf 'uso: sh tests/run.sh <caminho do ft>\n' >&2
    exit 2
}

[ $# -eq 1 ] || usage
[ -f "$1" ] && [ -x "$1" ] || usage

FT=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")
TESTS_DIR=$(cd "$(dirname "$0")" && pwd)/functional
export FT

ok=0
skipped=0
failed=0

for test in "$TESTS_DIR"/test_*.sh; do
    [ -f "$test" ] || continue
    name=$(basename "$test" .sh)
    work=$(mktemp -d "${TMPDIR:-/tmp}/ft_func_XXXXXX") || exit 2
    mkdir "$work/cwd" "$work/cap"

    (cd "$work/cwd" && CAP="$work/cap" FT_LIB="$TESTS_DIR/lib.sh" \
        sh -c '. "$FT_LIB" && . "$0"' "$test") >"$work/log" 2>&1
    status=$?

    case $status in
    0)
        ok=$((ok + 1))
        printf 'ok    %s\n' "$name"
        ;;
    77)
        skipped=$((skipped + 1))
        printf 'SKIP  %s: %s\n' "$name" "$(tail -n 1 "$work/log")"
        ;;
    *)
        failed=$((failed + 1))
        printf 'FAIL  %s\n' "$name"
        sed 's/^/      /' "$work/log"
        ;;
    esac

    if [ "${FT_TEST_KEEP:-0}" = 1 ]; then
        printf '      mantido em %s\n' "$work"
    else
        rm -rf "$work"
    fi
done

printf '%d ok, %d pulados, %d falhas\n' "$ok" "$skipped" "$failed"
[ "$failed" -eq 0 ]

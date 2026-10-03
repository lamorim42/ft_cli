# Erros de geração: mensagem em stderr e código 2 (spec project-generation).

# Fora do Linux, /proc/self/exe não existe e a ft não acha os templates (D6, passo 9).
if [ "$(uname -s)" != Linux ]; then
    ft_run new project foo
    assert_status 2
    assert_stdout_empty
    grep -q "^ft: não foi possível localizar os templates: " "$CAP/err" || {
        show_output
        fail "mensagem de templates não localizados ausente"
    }
    assert_not_exists foo
    exit 0
fi

# Diretório pai inexistente: para no primeiro mkdir, sem criar nada.
ft_run new project a/b
assert_status 2
assert_stdout_empty
assert_stderr <<'EOF'
ft: não foi possível criar o diretório a/b: No such file or directory
EOF
assert_not_exists a

# Template ilegível, numa cópia do ft com os próprios templates.
[ "$(id -u)" -ne 0 ] || skip "root ignora a permissão de leitura do template"

mkdir inst
cp "$FT" inst/ft
cp -R "$(dirname "$FT")/ft_templates" inst/ft_templates
chmod 000 inst/ft_templates/default/main.tpl

"$PWD/inst/ft" new project foo >"$CAP/out" 2>"$CAP/err"
FT_STATUS=$?
chmod 644 inst/ft_templates/default/main.tpl
assert_status 2
grep -q "^ft: não foi possível ler o template .*main\.tpl: Permission denied$" "$CAP/err" || {
    show_output
    fail "mensagem de template ilegível ausente"
}
if grep -qxF "criado  foo/app/src/main.c" "$CAP/out"; then
    show_output
    fail "main.c não deveria ter sido criado"
fi

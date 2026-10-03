# A6 e A7. A saída segue o SPEC §5.3; a árvore ainda não tem o .gitignore.
# Muda nos passos 6 (render) e 7 (.gitignore) do docs/TDD.md.

require_linux

# A6: ft new project minishell num diretório vazio
ft_run new project minishell
assert_status 0
assert_stderr_empty
assert_stdout <<'EOF'
criado  minishell/app/includes/minishell.h
criado  minishell/app/src/main.c
criado  minishell/Makefile
criado  minishell/README.md
Projeto 'minishell' criado com o template 'default'.
EOF

# Comportamento atual: sem .gitignore (D4, passo 7).
assert_tree minishell <<'EOF'
minishell
minishell/Makefile
minishell/README.md
minishell/app
minishell/app/bin
minishell/app/includes
minishell/app/includes/minishell.h
minishell/app/obj
minishell/app/src
minishell/app/src/main.c
minishell/app/tests
minishell/doc
minishell/doc/assets
EOF

# A6: conteúdo dos arquivos gerados
assert_file_has_line minishell/app/includes/minishell.h '#ifndef MINISHELL_H'
assert_file_has_line minishell/app/src/main.c '#include "minishell.h"'
assert_file_has_line minishell/app/src/main.c '    printf("Hello minishell!\n");'
assert_file_has_line minishell/Makefile 'NAME	= minishell'
printf '# Hello minishell' | assert_file_eq minishell/README.md

# A7: o projeto gerado compila sem warnings e roda
command -v clang >/dev/null 2>&1 || skip "clang não está no PATH (o makefile.tpl fixa CC = clang)"

cd minishell || fail "não entrou em minishell/"
make >"$CAP/make" 2>&1 || {
    cat "$CAP/make" >&2
    fail "make falhou no projeto gerado"
}
if grep -qi warning "$CAP/make"; then
    cat "$CAP/make" >&2
    fail "make emitiu warnings"
fi

./minishell >"$CAP/out" 2>"$CAP/err"
FT_STATUS=$?
assert_status 0
assert_stdout <<'EOF'
Hello minishell!
EOF

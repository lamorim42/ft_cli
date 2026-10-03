# Uso e comando desconhecido (snapshot do comportamento atual).
# Muda nos passos 2 (mensagens em stderr, ADR-0007) e 3 (help, A1, A2, A5).

# FALLBACK de app/include/files_contents.h
fallback()
{
    printf "Usage:\tft [option] [argument] [project_name]\n      \tTry 'ft --help' for more information.\n"
}

# ft sem argumentos (A1 pede o mesmo código, mas outra mensagem)
ft_run
assert_status 1
assert_stdout_empty
fallback | assert_stderr

# ft --help (D1: deveria mostrar a ajuda e sair com 0, passo 3)
ft_run --help
assert_status 1
assert_stdout_empty
fallback | assert_stderr

# ft foo a b (A5 pede "ft: comando desconhecido: foo" em stderr, passos 2 e 3)
ft_run foo a b
assert_status 1
assert_stderr_empty
assert_stdout <<'EOF'
Comando desconhecido: foo
EOF

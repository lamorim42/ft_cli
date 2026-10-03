# Uso e comando desconhecido.
# A mensagem de uso e o --help mudam no passo 3 (help, A1, A2).

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

# A5: ft foo a b
ft_run foo a b
assert_status 1
assert_stdout_empty
assert_stderr <<'EOF'
ft: comando desconhecido: foo
EOF
assert_tree . <<'EOF'
.
EOF

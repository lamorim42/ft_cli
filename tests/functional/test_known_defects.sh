# Defeitos conhecidos (snapshot do comportamento atual). Cada caso diz o defeito
# do docs/TDD.md §2.3 e o passo da migração que muda o esperado.

require_linux

# D3: repetir A6 sobrescreve o projeto sem aviso (passo 8, A8: código 2, nada alterado).
ft_run new project minishell
assert_status 0
printf 'trabalho do usuario\n' >minishell/README.md
ft_run new project minishell
assert_status 0
printf '# Hello minishell' | assert_file_eq minishell/README.md

# D2: nome inválido é aceito (passo 4, A10: código 1, nada criado).
ft_run new project meu-projeto
assert_status 0
assert_exists meu-projeto/app/includes/meu-projeto.h

# Tipo ignorado: só "project" deveria ser aceito (passo 4, A12: código 1).
ft_run new xyz foo
assert_status 0
assert_exists foo/Makefile

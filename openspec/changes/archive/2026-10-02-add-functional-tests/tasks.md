# Tasks

Critérios de aceite do `docs/SPEC.md` §9 cobertos, em versão snapshot do comportamento atual: **A6** (tarefa 2.1, sem `.gitignore`) e **A7** (tarefa 2.2). Nenhum código de produção muda (`skip_specs: true`).

## 1. Runner e helpers dos testes funcionais

- [x] 1.1 Criar `tests/functional/lib.sh` com `ft_run`, `assert_status`, `assert_stdout`, `assert_stderr`, `assert_stdout_empty`, `assert_stderr_empty`, `assert_tree`, `assert_file_eq`, `assert_exists`, `assert_not_exists`, `skip` e `fail` (design, decisão 3), só com POSIX sh; verificar com `sh -n tests/functional/lib.sh` e, se disponível, `dash -n` que não há erro de sintaxe
- [x] 1.2 Criar `tests/run.sh` que recebe o caminho do `ft`, exporta `FT` absoluto, roda cada `tests/functional/test_*.sh` num `mktemp -d` próprio (com `cwd/` e `cap/`), trata `0`/`77`/outro como ok/SKIP/FAIL, imprime o resumo, remove o diretório (exceto com `FT_TEST_KEEP=1`) e sai com `1` se houve falha; verificar com dois arquivos provisórios (`test_zz_pass.sh` com `exit 0`, `test_zz_fail.sh` com `fail "x"`) que o runner mostra `ok` e `FAIL`, sai com `1`, e com um terceiro (`skip "y"`) que aparece `SKIP`; depois remover os três
- [x] 1.3 Verificar que `tests/run.sh` sem argumento ou com um caminho que não é executável imprime o uso em `stderr` e sai com `2`, e que nenhum arquivo é criado no repositório depois de rodar o runner (`git status --porcelain` igual ao de antes)

## 2. Snapshot de `ft new project` (A6, A7)

- [x] 2.1 Criar `tests/functional/test_new_project.sh` com o caso A6: `skip` fora do Linux; `ft new project minishell` sai com `0`, `stderr` vazio, `stdout` igual às seis linhas do design (Context) e `assert_tree minishell` com a árvore atual sem `.gitignore`; verificar que `sh tests/run.sh ./ft` passa e que, trocando temporariamente uma linha do heredoc, o teste falha mostrando o diff
- [x] 2.2 No mesmo arquivo, acrescentar o conteúdo gerado (`#ifndef MINISHELL_H` em `minishell.h`; `#include "minishell.h"` e `Hello minishell!` em `main.c`; `NAME	= minishell` no `Makefile`; `README.md` igual a `# Hello minishell`) e o caso A7 (`skip` se não houver `clang`; `make` dentro de `minishell/` sai com `0` e a saída não contém `warning`; `./minishell` imprime exatamente `Hello minishell!` e sai com `0`); verificar que `sh tests/run.sh ./ft` passa

## 3. Snapshots de uso e de defeitos conhecidos

- [x] 3.1 Criar `tests/functional/test_usage.sh`: `ft` sem argumentos e `ft --help` saem com `1`, `stdout` vazio e `stderr` igual ao `FALLBACK` de `app/include/files_contents.h`; `ft foo a b` sai com `1` e `stdout` igual a `Comando desconhecido: foo`; verificar que `sh tests/run.sh ./ft` passa
- [x] 3.2 Criar `tests/functional/test_known_defects.sh` (`skip` fora do Linux), com um comentário em cada caso dizendo o defeito (D2, D3) e o passo do TDD que muda o esperado: repetir A6 depois de sobrescrever `minishell/README.md` sai com `0` e o `README.md` volta a `# Hello minishell`; `ft new project meu-projeto` sai com `0` e cria `meu-projeto/app/includes/meu-projeto.h`; `ft new xyz foo` sai com `0` e cria `foo/Makefile`; verificar que `sh tests/run.sh ./ft` passa com os três arquivos de teste

## 4. Testes unitários complementares

- [x] 4.1 Em `tests/unit/test_fs.c`, acrescentar (com `RUN_TEST`): `fs_create_dir` com pai inexistente retorna `-1` e o caminho não existe; `fs_create_dir` sobre arquivo comum retorna `-1`; com `umask(022)`, o diretório criado tem modo `0755`; `fs_create_file` em diretório inexistente retorna `-1`; `fs_read_file` devolve exatamente um arquivo de 64 KiB e um arquivo sem `\n` final; verificar que `make test-unit` passa
- [x] 4.2 Em `tests/unit/test_parser.c`, acrescentar: `-h` com `argc` 2 retorna `1`; `ctx.command`, `ctx.type` e `ctx.project_name` são os próprios ponteiros de `argv[1..3]`; `ft new project --force` preenche `project_name` com `"--force"` (comentário: muda no passo 3); verificar que `make test-unit` passa
- [x] 4.3 Em `tests/unit/test_dispatcher.c`, acrescentar: `"NEW"` retorna `1` e `""` retorna `1`, sem incluir `command.h`; verificar que `make test-unit` passa
- [x] 4.4 Formatar os `.c` alterados com o `.clang-format` do repositório; verificar com `clang-format --dry-run --Werror tests/unit/*.c` (ou `pipx run clang-format ...`) que não há diferenças

## 5. Makefile e documentação

- [x] 5.1 No `Makefile`, adicionar `FUNC_RUNNER` e o alvo `test-functional` (dependendo de `$(NAME)`), trocar `test` pela receita que roda `test-unit` e `test-functional` e só falha no fim, e incluir `test-functional` no `.PHONY` (design, decisão 6); verificar que num clone limpo `make test-functional` compila o `./ft` e passa, e que, com um `tests/unit/test_zz_fail.c` provisório que falha, `make test` ainda roda os funcionais e sai com código diferente de zero (depois remover o arquivo)
- [x] 5.2 Atualizar a seção "Testes" do `README.md`: `make test`, `make test-unit`, `make test-functional`, `FT_TEST_KEEP=1`, onde ficam os testes funcionais, como criar um `test_<cenário>.sh` (exemplo curto com `ft_run` e `assert_*`), o protocolo `0`/`77`/falha, e o aviso de que os testes registram o comportamento atual e são atualizados por cada passo da migração; verificar que os comandos descritos funcionam num clone limpo
- [x] 5.3 Atualizar `docs/adr/0008-testes-e-ci.md`: registrar que a camada de integração do TDD se chama "funcional" no repositório (`tests/run.sh` + `tests/functional/`), o protocolo dos testes e que os primeiros são snapshots do comportamento atual (A6, A7, D1–D3); verificar que o ADR não contradiz o `design.md` deste change

## 6. Verificação final

- [x] 6.1 Num clone limpo, rodar `make test`; verificar que compila sem avisos, roda os três executáveis unitários e os três arquivos funcionais, todos passam (ou aparecem como `SKIP` com motivo, fora do Linux) e o código de saída é `0`
- [x] 6.2 Rodar `make fclean && make test CFLAGS="-Wall -Wextra -Werror -g -fsanitize=address,undefined"`; verificar que não há erros de memória nem comportamento indefinido nos unitários nem no `./ft` chamado pelos funcionais (registrar no PR qualquer vazamento conhecido do código atual, sem corrigi-lo aqui)
- [x] 6.3 Depois de `make test && make fclean`, verificar que `git status --porcelain` não mostra arquivos gerados pelos testes

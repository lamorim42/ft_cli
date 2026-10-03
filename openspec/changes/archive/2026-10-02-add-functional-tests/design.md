# Design

## Context

Ver `proposal.md` (Why) para a motivação.

**Passo do TDD:** este change implementa o **passo 1** do `docs/TDD.md` §7:

| # | Passo | Resolve | Testes que entram |
|---|---|---|---|
| 1 | `tests/run.sh` com A6 e A7 (snapshot do comportamento atual) | — | A6, A7 |

"Os passos 1 e 3 vêm primeiro de propósito: o teste de snapshot protege tudo o que vem depois, e o `--help` é a regressão mais visível."

**ADRs relacionados:** ADR-0008 (estratégia de testes: unitários com Unity e integração em shell, `tests/run.sh` chamado por `make test`), ADR-0007 (mensagens em `stderr`, códigos de saída; os testes registram o estado anterior a ele), ADR-0006 e ADR-0001 (os defeitos D1–D3 que os snapshots registram).

**Trecho do TDD §6 que este change implementa** (copiado e ajustado ao estado atual):

| Camada | Ferramenta no TDD | Neste change |
|---|---|---|
| Unitário | Executáveis C em `tests/unit/` (o change anterior trocou `assert` por Unity) | Novos casos de borda em `test_fs.c`, `test_parser.c` e `test_dispatcher.c` |
| Integração | `tests/run.sh` (POSIX sh). "Os critérios A1–A20 do SPEC, cada um num `mktemp -d` próprio" | `tests/run.sh` + `tests/functional/test_*.sh`, cada arquivo num `mktemp -d` próprio. Só A6 e A7, em versão snapshot, mais os casos atuais de erro |
| Memória | `make sanitize && make test` | Fora: passo 12. Verificado à mão com `CFLAGS` extras (tasks) |

O TDD chama a camada de "integração"; aqui ela se chama **funcional** (`tests/functional/`, `make test-functional`), que é o nome usado pelo time. O ADR-0008 registra a equivalência.

**Estado atual relevante** (verificado rodando `./ft` num diretório temporário):

- `cmd_new_execute` acha os templates por `readlink("/proc/self/exe")` + `/ft_templates/default/`. Logo, o binário testado precisa ser o `./ft` da raiz do repositório, chamado por caminho absoluto, e a geração só funciona no Linux (D6).
- `ft new project minishell` sai com `0`, imprime em `stdout`:
  ```
  Executando comando NEW para projeto minishell
  Criado arquivo: minishell/app/includes/minishell.h
  Criado arquivo: minishell/app/src/main.c
  Criado arquivo: minishell/Makefile
  Criado arquivo: minishell/README.md
  Estrutura criada com sucesso em minishell!
  ```
  e cria `app/{bin,includes,obj,src,tests}`, `doc/assets`, os quatro arquivos acima e **nenhum** `.gitignore` (D4).
- Dentro de `minishell/`, `make` compila com `clang -Wall -Werror -Wextra` sem avisos, e `./minishell` imprime `Hello minishell!` e sai com `0`. O `makefile.tpl` fixa `CC = clang`.
- Repetir o comando sai com `0`, imprime nove vezes `Error creating directory: File exists` em `stderr` (via `perror`) e regrava os quatro arquivos (D3).
- `ft` sem argumentos e `ft --help` saem com `1` e escrevem o `FALLBACK` de `files_contents.h` em `stderr` (D1).
- `ft foo a b` sai com `1` e escreve `Comando desconhecido: foo` em `stdout` (ADR-0007 muda isso no passo 2).
- `ft new project meu-projeto` sai com `0` e cria `meu-projeto/` (D2); `ft new xyz foo` ignora o tipo e cria `foo/`.
- `make test` hoje é `test: test-unit`, um laço que roda `tests/bin/test_*` e falha no fim.

## Goals / Non-Goals

**Goals:**

- Um teste funcional novo exige só criar `tests/functional/test_<cenário>.sh`, sem mexer no `Makefile` nem no `run.sh`.
- Cada teste roda isolado: diretório temporário próprio, nunca escreve no repositório, e uma falha não impede os outros de rodar.
- Uma falha mostra o que diferiu (diff entre esperado e obtido), não só "falhou".
- Só `sh` POSIX e utilitários POSIX (`mktemp`, `find`, `sort`, `diff`, `grep`). Nada para instalar.

**Non-Goals:**

- Framework de shell externo (bats, shunit2).
- Paralelizar os testes.
- Testar `make install` (escreve em `/usr/local`; o `DESTDIR` chega no passo 9).

## Decisions

### 1. Shell POSIX para os testes funcionais, Unity só para os unitários

O que os testes funcionais verificam é a interface de processo da `ft`: código de saída, `stdout`, `stderr` e a árvore no disco, e depois rodar `make` no projeto gerado. Em shell isso é uma linha cada (`"$FT" new project x >out 2>err; echo $?`, `find | sort`, `diff`).

Alternativas consideradas:

- **C com Unity, fazendo `fork`/`exec` do `./ft`:** uma ferramenta só, mas exige escrever captura de `stdout`/`stderr` por pipe, comparação de árvore e chamada de `make` em C. É muito código de apoio para o que o shell faz nativamente, e foi rejeitado pelo ADR-0008.
- **bats ou shunit2:** relatórios melhores, mas são dependências a instalar ou copiar. Os helpers de que precisamos cabem em ~80 linhas de `lib.sh`.

### 2. Layout e protocolo dos testes

```
tests/
  run.sh                 runner: recebe o caminho do ft, roda tests/functional/test_*.sh
  functional/
    lib.sh               helpers (ft_run, assert_*, skip, fail)
    test_new_project.sh  A6 e A7 (snapshot) e conteúdo dos arquivos gerados
    test_usage.sh        sem argumentos, --help, comando desconhecido
    test_known_defects.sh  D2, D3 e tipo ignorado (comportamento atual)
```

**Protocolo** (inspirado no do automake, para não inventar um):

- `tests/run.sh ./ft` converte o caminho em absoluto, confere que é executável e exporta `FT`.
- Para cada `tests/functional/test_*.sh` (em ordem alfabética), cria `work=$(mktemp -d)` com `work/cwd/` e `work/cap/`, entra em `work/cwd/` e roda o arquivo com `sh` num subprocesso.
- Código de saída do arquivo: `0` passou, `77` pulado (com o motivo impresso), qualquer outro falhou.
- O runner imprime uma linha por arquivo (`ok`, `SKIP` ou `FAIL`), um resumo `N ok, N pulados, N falhas` e sai com `1` se houve falha.
- `work/` é removido ao final de cada arquivo, a não ser que `FT_TEST_KEEP=1` esteja definido (para depurar), caso em que o caminho é impresso.

Um arquivo de teste é uma sequência de chamadas, sem funções registradas: a primeira asserção que falha encerra o arquivo com `1`. Por isso cada arquivo agrupa um tema, e cenários independentes vão em arquivos separados.

### 3. Helpers de `lib.sh`

| Helper | O que faz |
|---|---|
| `ft_run args...` | Roda `"$FT" args...` com `stdout` em `$CAP/out` e `stderr` em `$CAP/err`, e guarda o código em `$FT_STATUS` |
| `assert_status N` | Falha se `$FT_STATUS` ≠ `N`, mostrando `stdout` e `stderr` |
| `assert_stdout` / `assert_stderr` | Compara a saída com o texto exato lido do `stdin` (heredoc), via `diff -u` |
| `assert_stdout_empty` / `assert_stderr_empty` | Saída vazia |
| `assert_tree dir` | Compara `find dir \| LC_ALL=C sort` com o `stdin` |
| `assert_file_eq path` | Compara o conteúdo do arquivo com o `stdin` |
| `assert_exists path` / `assert_not_exists path` | Existência |
| `skip motivo` / `fail motivo` | `exit 77` / `exit 1` com a mensagem |

`$CAP` aponta para `work/cap/`, fora do diretório onde a `ft` roda, para que os arquivos de captura não apareçam na árvore comparada.

`LC_ALL=C` no `sort` garante a mesma ordem no Linux e no macOS.

### 4. Casos funcionais

Todos registram o comportamento **atual**; os marcados com um passo serão atualizados por ele.

| Arquivo | Caso | Esperado hoje | Muda no passo |
|---|---|---|---|
| `test_new_project.sh` | A6: `ft new project minishell` em diretório vazio | código `0`, `stdout` exato (Context), `stderr` vazio, árvore exata sem `.gitignore` | 2 (mensagens), 7 (`.gitignore`) |
| | A6: conteúdo | `minishell.h` com `#ifndef MINISHELL_H`; `main.c` com `#include "minishell.h"` e `Hello minishell!`; `Makefile` com `NAME	= minishell`; `README.md` igual a `# Hello minishell` | 6 (render) |
| | A7: `cd minishell && make && ./minishell` | `make` sai com `0` sem a palavra `warning`; `./minishell` imprime `Hello minishell!` e sai com `0` | — |
| `test_usage.sh` | `ft` sem argumentos | código `1`, `stdout` vazio, `stderr` igual ao `FALLBACK` | 2, 3 (A1) |
| | `ft --help` (D1) | igual ao anterior | 3 (A2) |
| | `ft foo a b` | código `1`, `stdout` = `Comando desconhecido: foo` | 2, 3 (A5) |
| `test_known_defects.sh` | D3: repetir A6 depois de editar `README.md` | código `0`, `README.md` volta a `# Hello minishell` | 8 (A8) |
| | D2: `ft new project meu-projeto` | código `0`, `meu-projeto/app/includes/meu-projeto.h` existe | 4 (A10) |
| | tipo ignorado: `ft new xyz foo` | código `0`, `foo/Makefile` existe | 4 (A12) |

**Plataforma:** os casos que geram projeto chamam `skip` quando `uname -s` não é `Linux` (D6, `/proc/self/exe`; resolvido no passo 9). O A7 chama `skip` se `clang` não estiver no `PATH`, porque o `makefile.tpl` atual fixa `CC = clang`. Os casos de `test_usage.sh` rodam em qualquer plataforma.

Os casos destrutivos (`../x`, `a/b`) não entram: hoje escapariam do diretório temporário (D2). Eles chegam com a validação do passo 4, que os rejeita antes de tocar o disco.

### 5. Casos unitários complementares

Mesmo critério do change anterior: só funções públicas, comportamento atual, sem verificar mensagens impressas (o formato muda no passo 2).

| Arquivo | Casos novos | Muda no passo |
|---|---|---|
| `test_fs.c` | `fs_create_dir` com pai inexistente retorna `-1` e não cria nada (sem `mkdir -p`); `fs_create_dir` sobre um arquivo comum retorna `-1`; diretório criado tem modo `0755` com `umask(022)`; `fs_create_file` em diretório inexistente retorna `-1`; `fs_read_file` devolve byte a byte um arquivo de 64 KiB e um arquivo sem `\n` final | 5 |
| `test_parser.c` | `-h` com `argc` 2 retorna `1`; os campos apontam para o próprio `argv` (sem cópia); com 4 argumentos, uma flag na posição do nome vira `project_name` (`ft new project --force` → `"--force"`) | 3 |
| `test_dispatcher.c` | o nome do comando diferencia maiúsculas (`"NEW"` retorna `1`); comando vazio `""` retorna `1` | 3 |

O caminho `dispatch_command` → `cmd_new_execute` continua fora dos unitários: o `cmd_new` acha os templates a partir do executável, que no teste seria `tests/bin/`. Ele é coberto pelos funcionais.

### 6. Integração com o Makefile

```make
FUNC_RUNNER = $(TEST_DIR)/run.sh

test-functional: $(NAME)
	@sh $(FUNC_RUNNER) ./$(NAME)

test:
	@fail=0; \
	$(MAKE) --no-print-directory test-unit || fail=1; \
	$(MAKE) --no-print-directory test-functional || fail=1; \
	exit $$fail

.PHONY: test test-unit test-functional
```

- **`test` roda as duas suítes e só falha no fim.** Com `test: test-unit test-functional`, o `make` pararia na primeira dependência que falha e o resultado da outra suíte não apareceria.
- **`test-functional` depende de `$(NAME)`**, então `make test-functional` num clone limpo compila o `./ft` antes.
- **`sh $(FUNC_RUNNER)`** em vez de depender do bit de execução, que pode se perder em alguns checkouts.

### 7. Estilo

- Os `.sh` usam só POSIX sh (sem `[[`, arrays, `local` ou `$'...'`) e começam com `#!/bin/sh` e `set -u`. Não usam `set -e`, porque os testes precisam ver códigos de saída diferentes de zero da `ft`.
- Os `.c` alterados seguem o `.clang-format`.

## Risks / Trade-offs

- [Snapshots de mensagens quebram em todo passo que mexe em texto] → Esperado: o passo 2 troca as mensagens e atualiza os heredocs no mesmo PR, e o diff do teste mostra a mudança de comportamento na revisão.
- [No macOS quase todos os casos de geração são pulados até o passo 9] → O runner imprime `SKIP` com o motivo, e o resumo conta os pulados, para que "verde" no macOS não pareça cobertura total.
- [Um teste com bug pode escrever fora do `mktemp -d`] → Os testes só chamam a `ft` com nomes simples, e os casos `../x` e `a/b` ficam fora até o passo 4.
- [`make` dentro do projeto gerado depende do `clang` local] → `skip` explícito, e o `makefile.tpl` passa a respeitar `CC` num change futuro.
- [Saída ruidosa: a `ft` e os unitários imprimem mensagens próprias] → As capturas vão para `$CAP`; o runner só mostra a saída de um teste que falhou.
- [Esquecer de atualizar um snapshot faz o passo seguinte falhar] → É exatamente a rede de segurança pedida.

## Migration Plan

Não há migração: o change só adiciona testes e altera alvos de teste no `Makefile`. `make`, `make install` e o binário `ft` não mudam. Rollback é reverter o commit.

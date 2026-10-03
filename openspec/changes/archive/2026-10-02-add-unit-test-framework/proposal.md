# Proposal

## Why

A refatoração planejada no `docs/TDD.md` vai reescrever parser, registro de comandos, módulo `fs` e motor de templates, e hoje não existe nenhum teste automatizado (`tests/` está vazio). Sem uma base de testes unitários, cada passo da migração só pode ser validado à mão, e foi assim que as regressões da branch `feature/refatorando` passaram despercebidas (ADR-0008). Escolher a biblioteca agora evita que cada change seguinte reabra essa decisão.

## What Changes

- Escolha e adoção de uma biblioteca de testes unitários em C, avaliada por três critérios: acessível (sem instalação nem `sudo`, funciona em Linux e macOS), fácil de usar e flexível para a estrutura atual (Makefile simples, módulos em `app/src/<módulo>/`). A recomendação é a **Unity** (ThrowTheSwitch), copiada para o repositório; a comparação está no `design.md`.
- Estrutura de testes em `tests/`: biblioteca em `tests/vendor/unity/`, testes em `tests/unit/test_<módulo>.c`, um executável por arquivo de teste.
- Novos alvos no `Makefile`: `make test` (compila e roda todos os testes unitários e falha se algum falhar) e `make test-unit`. O `make test` fica preparado para incluir os testes de integração em shell (`tests/run.sh`, ADR-0008) num change futuro.
- Primeiros testes unitários, que registram o comportamento **atual** das funções públicas que já existem: `parse_args`, `dispatch_command`, `fs_read_file`, `fs_create_dir` e `fs_create_file`. Esses testes servem de rede de segurança e serão atualizados pelos changes que reescreverem essas funções (passos 3b e 5 do TDD).
- `fclean` remove os binários e objetos de teste, e o `.gitignore` passa a ignorá-los.
- ADR-0008 atualizado com a biblioteca escolhida.

Fora do escopo:

- Testes de integração em shell (A6/A7 e casos negativos): ficam para o change seguinte.
- Extrair funções `static` de `cmd_new.c` (`str_to_upper`, `replace_placeholder`) só para testá-las: elas serão substituídas pelo `tpl_render` (passo 6 do TDD), que já nasce com testes.
- CI e sanitizers no pipeline (passo 12 do TDD).
- Mudança de comportamento da `ft`: nenhuma.

## Capabilities

### New Capabilities

Nenhuma. O change não altera comportamento observável da `ft` e declara `skip_specs: true`.

### Modified Capabilities

Nenhuma.

## Impact

- **Código de produção:** nenhuma alteração em `app/src/`.
- **Build:** `Makefile` ganha os alvos de teste e a separação entre os objetos da aplicação e o `main.o`, para que os testes linkem o código sem o `main` da `ft`.
- **Dependências:** Unity (licença MIT) copiada em `tests/vendor/unity/`. Nenhuma instalação no sistema.
- **Repositório:** novos diretórios `tests/unit/` e `tests/vendor/`. `.gitignore` atualizado.
- **Documentação:** `docs/adr/0008-testes-e-ci.md` e o `README.md` (seção de como rodar os testes).

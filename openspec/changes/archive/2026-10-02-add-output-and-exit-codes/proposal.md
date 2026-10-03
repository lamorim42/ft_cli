# Proposal

## Why

Hoje a `ft` não tem convenção de saída: o `dispatcher` escreve o erro de comando desconhecido em `stdout`, `fs_create_dir` imprime `Error creating directory` em inglês via `perror`, `cmd_new` mistura `printf` e `fprintf(stderr, ...)`, e toda falha sai com código `1`, seja erro de uso, de disco ou de memória. Scripts não conseguem distinguir os casos, e os próximos passos da migração (parser, validação, gerador) precisam de um lugar único para emitir mensagens e de códigos de saída definidos. Este change é o passo 2 do `docs/TDD.md` §7 e aplica o ADR-0007.

## What Changes

- **Códigos de saída padronizados** num header próprio: `0` sucesso, `1` erro de uso, `2` erro de sistema de arquivos, `3` erro interno (memória ou erro inesperado). O código de saída de cada falha atual passa a seguir essa tabela.
- **Módulo de saída** único para mensagens: erros e avisos em `stderr` com o prefixo `ft: ` (e `ft: aviso: `), saída normal em `stdout`. O código ativo deixa de chamar `printf`/`perror` diretamente.
- **Comando desconhecido** (A5): `ft foo a b` escreve `ft: comando desconhecido: foo` em `stderr` e sai com `1`. **BREAKING** para quem lia a mensagem em `stdout`.
- **Saída de sucesso do `ft new project`** no formato do SPEC §5.3: uma linha `criado  <caminho>` por arquivo e, no fim, `Projeto '<nome>' criado com o template 'default'.`. A linha `Executando comando NEW...` sai. **BREAKING** para quem dependia do texto antigo.
- **Erros de geração em português**, em `stderr`, com o caminho e o motivo do sistema, e código `2`: template ilegível, arquivo ou diretório que não pode ser criado.
- **`fs_*` deixam de imprimir mensagens** e só retornam o erro com `errno` preservado. Um diretório que já existe continua sendo aceito em silêncio (a proteção contra sobrescrita é o passo 8).
- **D5 parcial:** falhas de alocação no `cmd_new` (`str_to_upper`, `replace_placeholder`) passam a ser checadas, com `ft: memória insuficiente` e código `3`, em vez de seguir com `NULL`.
- Testes unitários e funcionais atualizados para o novo formato, e novos casos para os códigos `1` e `2`.

Fora do escopo:

- Texto da mensagem de uso (`ft` sem argumentos, `ft --help`): continua o atual, já em `stderr` com código `1`. O passo 3 traz o `help` e o uso definitivos.
- `--verbose`: a função de depuração existe, mas nada liga o modo verboso até o parser novo (passo 3).
- `.gitignore` na saída de sucesso (passo 7), "já existe" com código `2` (passo 8), validação de nome e tipo (passo 4).
- Código legado sem uso (`cli/*`, `core/*`, `cli_utils.c`): continua como está até a remoção no passo 11.

## Capabilities

### New Capabilities

- `cli`: interface de linha de comando da `ft`: códigos de saída, para onde vão as mensagens e o tratamento de comando desconhecido.
- `project-generation`: o comando `ft new project`: o que imprime ao gerar um projeto e como reporta falhas de geração.

### Modified Capabilities

Nenhuma (`openspec/specs/` ainda está vazio).

## Impact

- **Código:** novos `app/include/ft.h`, `app/include/output.h` e `app/src/output/output.c`; alterados `main.c`, `dispatcher.c`, `cmd_new.c`, `fs_create_dir.c`, `fs_create_file.c`; `Makefile` ganha o novo fonte.
- **Comportamento visível:** mensagens de `stdout`/`stderr` e códigos de saída de `ft new project` e de comando desconhecido.
- **Testes:** `tests/functional/test_new_project.sh`, `test_usage.sh`, `test_known_defects.sh` atualizados; novo `tests/functional/test_errors.sh`; novo `tests/unit/test_output.c`; `test_dispatcher.c` e `test_fs.c` ganham casos.
- **Documentação:** ADR-0007 continua Proposto (estilo e includes ainda pendentes); o README não descreve mensagens e não muda.

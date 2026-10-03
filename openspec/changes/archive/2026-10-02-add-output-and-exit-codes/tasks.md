# Tasks

Critérios de aceite do `docs/SPEC.md` §9 cobertos: **A5** (tarefas 2.3 e 4.2), **A1** parcial, só destino e código (tarefa 4.2), e **A6** parcial, saída de sucesso sem `.gitignore` (tarefa 3.4). Os demais cenários das specs (diretório inexistente, template ilegível, templates não localizados) são cobertos pela tarefa 4.4, e a falta de memória pela revisão da tarefa 3.5.

## 1. Códigos de saída e módulo de saída

- [x] 1.1 Criar `app/include/ft.h` com o enum `e_ft_status` (`FT_OK`, `FT_EUSAGE`, `FT_EIO`, `FT_EINTERNAL`) do design (decisão 2); verificar que um `.c` que só inclui `ft.h` compila com `-Wall -Wextra -Werror`
- [x] 1.2 Criar `app/include/output.h` e `app/src/output/output.c` com `ft_err`, `ft_warn`, `ft_info`, `ft_debug` e `g_verbose` (decisão 1: `\n` acrescentado, `format(printf)`, `fflush(stdout)` antes de escrever em `stderr`, `errno` preservado), sem incluir nenhum header do projeto além de `output.h`; adicionar o fonte ao `SRC` do `Makefile`; verificar que `make re` compila sem avisos
- [x] 1.3 Criar `tests/unit/support/capture.h` e `capture.c` para capturar `stdout` ou `stderr` de um trecho de código num arquivo temporário e devolver o texto (decisão 5); verificar com o primeiro teste da tarefa 1.4 que a captura funciona e que os descritores são restaurados (um `printf` depois da captura aparece na saída do teste)
- [x] 1.4 Criar `tests/unit/test_output.c`: `ft_err("x %d", 42)` escreve exatamente `ft: x 42\n` em `stderr` e nada em `stdout`; `ft_warn` usa `ft: aviso: `; `ft_info` escreve em `stdout` sem prefixo; `ft_debug` não escreve com `g_verbose = 0` e escreve `ft: debug: ...` com `g_verbose = 1`; `errno` é igual antes e depois de `ft_err`; verificar que `make test-unit` passa

## 2. Dispatcher, main e fs

- [x] 2.1 Em `fs_create_dir.c` e `fs_create_file.c`, remover o `perror` e manter o retorno `-1` com `errno` do sistema; verificar com novos casos em `tests/unit/test_fs.c` (usando `capture`) que as falhas não escrevem em `stderr` e que `fs_create_dir` sobre diretório existente deixa `errno == EEXIST`, e que `make test-unit` passa
- [x] 2.2 Em `main.c`, retornar `FT_EUSAGE` quando `parse_args` falha, mantendo o `dprintf` do `FALLBACK` (decisão 4); verificar que `test_usage.sh` continua passando para `ft` e `ft --help`
- [x] 2.3 Em `dispatcher.c`, trocar o `printf` por `ft_err("comando desconhecido: %s", ...)` e retornar `FT_EUSAGE`; atualizar `tests/unit/test_dispatcher.c` para conferir o código, `stderr` igual a `ft: comando desconhecido: foo\n` e `stdout` vazio nos três casos de comando desconhecido (A5); verificar que `make test-unit` passa

## 3. `cmd_new` com mensagens e códigos novos

- [x] 3.1 Em `cmd_new.c`, remover a linha `Executando comando NEW...` e tratar a falha do `readlink` com `ft_err("não foi possível localizar os templates: %s", strerror(errno))` e `FT_EIO`, e a de `str_to_upper` com `ft_err("memória insuficiente")` e `FT_EINTERNAL`; verificar que `make` compila sem avisos
- [x] 3.2 Fazer `create_project_dir` retornar um código: `EEXIST` segue em silêncio, qualquer outro erro emite `não foi possível criar o diretório <caminho>: <motivo>` e retorna `FT_EIO`; em `cmd_new_execute`, checar o retorno da criação de `<nome>` e de cada subdiretório e parar no primeiro erro; verificar à mão que `ft new project a/b` num diretório vazio sai com `2` e a mensagem da spec
- [x] 3.3 Em `process_template`, usar as mensagens e códigos da decisão 3 para `fs_read_file` `NULL` (`FT_EIO`), `replace_placeholder` `NULL` nas duas chamadas (`FT_EINTERNAL`, liberando o que já foi alocado) e `fopen` falho (`FT_EIO`); fazer `process_templates` propagar o código; verificar que `make` compila sem avisos
- [x] 3.4 Trocar `Criado arquivo: ...` por `ft_info("criado  %s", ...)` e `Estrutura criada com sucesso...` por `ft_info("Projeto '%s' criado com o template 'default'.", ...)`, retornando `FT_OK`; atualizar o `stdout` esperado em `tests/functional/test_new_project.sh` para o da spec `project-generation` (A6); verificar que `sh tests/run.sh ./ft` passa
- [x] 3.5 Revisar `cmd_new.c` e confirmar que toda alocação (`str_to_upper`, as duas `replace_placeholder`) é checada antes do uso e que cada caminho de erro libera o que já alocou; verificar com `grep -nE 'printf|perror|puts' app/src/main.c app/src/dispatcher app/src/commands app/src/fs app/src/parser` que só restam o `dprintf` do `FALLBACK` e os `snprintf` de montagem de caminho

## 4. Testes funcionais

- [x] 4.1 Formatar os `.c` e `.h` novos ou alterados com o `.clang-format`; verificar com `clang-format --dry-run --Werror` (ou `pipx run clang-format ...`) nos arquivos tocados que não há diferenças
- [x] 4.2 Atualizar `tests/functional/test_usage.sh`: `ft foo a b` com `stdout` vazio, `stderr` igual a `ft: comando desconhecido: foo` e código `1` (A5); `ft` e `ft --help` sem mudança (A1 parcial); verificar que `sh tests/run.sh ./ft` passa
- [x] 4.3 Atualizar `tests/functional/test_known_defects.sh`: repetir A6 sai com `0` e agora com `stderr` vazio; atualizar o comentário do caso; verificar que `sh tests/run.sh ./ft` passa
- [x] 4.4 Criar `tests/functional/test_errors.sh` com os três casos da decisão 5: `ft new project a/b` (código `2`, mensagem exata, `a` não existe); template ilegível numa cópia de `ft` + `ft_templates/` com `main.tpl` em `chmod 000` (código `2`, linha de erro da spec, sem `criado  foo/app/src/main.c`; `skip` como root); e, fora do Linux, `ft new project foo` com `ft: não foi possível localizar os templates: ...`, código `2` e `foo` inexistente; verificar que `sh tests/run.sh ./ft` passa no Linux

## 5. Verificação final

- [x] 5.1 Num clone limpo, rodar `make test`; verificar que compila sem avisos, todos os unitários e funcionais passam e o código de saída é `0`
- [x] 5.2 Rodar `make fclean && make test CFLAGS="-Wall -Wextra -Werror -g -fsanitize=address,undefined"`; verificar que não há erros de memória nem comportamento indefinido
- [x] 5.3 Rodar `openspec validate add-output-and-exit-codes --strict` e conferir à mão cada cenário das specs `cli` e `project-generation` contra um teste das tarefas acima (ou a revisão da 3.5); verificar que nenhum cenário ficou sem cobertura

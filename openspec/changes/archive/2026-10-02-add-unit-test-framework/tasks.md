# Tasks

Nenhum critério de aceite do `docs/SPEC.md` §9 é coberto por este change: ele não altera comportamento da `ft` (`skip_specs: true`).

## 1. Unity no repositório

- [x] 1.1 Copiar `unity.c`, `unity.h` e `unity_internals.h` da última release estável 2.6.x de ThrowTheSwitch/Unity para `tests/vendor/unity/`, com `LICENSE.txt` e um arquivo `VERSION` contendo a tag usada; verificar que os cinco arquivos existem e que `VERSION` corresponde a uma tag publicada
- [x] 1.2 Criar `tests/vendor/.clang-format` com `DisableFormat: true`; verificar com `clang-format --dry-run tests/vendor/unity/unity.c` (se o `clang-format` estiver instalado) que nenhuma alteração é proposta
- [x] 1.3 Compilar `tests/vendor/unity/unity.c` isoladamente com `clang -Wall -Wextra -Werror -c`; verificar que compila sem avisos ou, se houver aviso, registrar a flag mínima que o desliga só para `unity.c` (design, decisão 3)

## 2. Build dos testes

- [x] 2.1 No `Makefile`, definir `APP_OBJS` (os objetos da aplicação sem `main.o`) e as variáveis `TEST_DIR`, `UNITY_DIR`, `TEST_SRC`, `TEST_SUP`, `TEST_BINS` e `TEST_FLAGS` do design; verificar que `make` e `make re` continuam gerando o `./ft` como antes
- [x] 2.2 Adicionar a regra `tests/bin/%` e os alvos `test-unit` e `test` (com `.PHONY` próprio), rodando todos os executáveis e falhando no fim se algum falhar; verificar com um `tests/unit/test_smoke.c` provisório (um teste que passa e outro que falha) que `make test` roda os dois, mostra a falha e sai com código diferente de zero, e depois remover o teste que falha
- [x] 2.3 Fazer `fclean` remover `tests/bin/` e `tests/obj/` e adicionar as duas pastas ao `.gitignore`; verificar que, depois de `make test && make fclean`, `git status` não mostra nada gerado

## 3. Apoio para testes de sistema de arquivos

- [x] 3.1 Criar `tests/unit/support/tmpdir.h` e `tmpdir.c` com uma função que cria um diretório temporário via `mkdtemp` (respeitando `$TMPDIR`) e outra que o remove recursivamente; verificar com um teste em `test_smoke.c` que o diretório existe depois da criação e não existe depois da remoção, mesmo contendo arquivos e subdiretórios

## 4. Primeiros testes unitários (comportamento atual)

- [x] 4.1 Criar `tests/unit/test_parser.c` cobrindo `parse_args`: 4 argumentos preenchem `command`, `type` e `project_name` e zeram `verbose`; `argc` 1, 2, 3 e 5 retornam `1`; verificar que `make test` passa
- [x] 4.2 Criar `tests/unit/test_dispatcher.c` cobrindo `dispatch_command` com comando desconhecido (retorna `1`), sem incluir `command.h`; verificar que `make test` passa
- [x] 4.3 Criar `tests/unit/test_fs.c` cobrindo `fs_create_dir` (cria; retorna `-1` se já existe), `fs_create_file` (cria vazio; trunca existente) e `fs_read_file` (conteúdo exato; arquivo vazio vira `""`; inexistente retorna `NULL`), cada teste dentro de um diretório de `tmpdir`; verificar que `make test` passa e que nenhum arquivo é criado fora do diretório temporário
- [x] 4.4 Remover `tests/unit/test_smoke.c` (o teste de `tmpdir` passa a ser coberto indiretamente por `test_fs.c`); verificar que `make test` passa só com os três arquivos de teste
- [x] 4.5 Formatar os arquivos de `tests/unit/` com o `.clang-format` do repositório; verificar com `clang-format --dry-run --Werror tests/unit/*.c tests/unit/support/*` (se instalado) que não há diferenças

## 5. Documentação

- [x] 5.1 Adicionar ao `README.md` uma seção "Testes" explicando `make test`, onde ficam os testes, como criar um `test_<módulo>.c` novo (incluindo a regra de um `RUN_TEST` por função de teste) e que a Unity está em `tests/vendor/`; verificar que os comandos descritos funcionam num clone limpo
- [x] 5.2 Atualizar `docs/adr/0008-testes-e-ci.md`: registrar a Unity como biblioteca dos testes unitários, com a comparação resumida (Criterion, cmocka, acutest, `assert` puro) e o motivo da escolha, e ajustar a alternativa "Usar só testes unitários em C" para deixar claro que unitários e integração convivem; verificar que o ADR não contradiz o `design.md` deste change

## 6. Verificação final

- [x] 6.1 Num clone limpo, rodar `make test`; verificar que compila sem avisos, executa `test_parser`, `test_dispatcher` e `test_fs`, todos passam e o código de saída é `0`
- [x] 6.2 Rodar `make test CFLAGS="-Wall -Wextra -Werror -g -fsanitize=address,undefined"`; verificar que não há erros de memória nem comportamento indefinido nos testes

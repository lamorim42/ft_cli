# Design

## Context

Ver `proposal.md` (Why) para a motivação.

**Passo do TDD:** este change não corresponde a um passo exato do `docs/TDD.md` §7. Ele antecipa a infraestrutura de testes unitários que o passo 6 ("`strbuf` e `tpl_render`, com testes unitários") pressupunha, para que os passos 1 a 12 já possam acrescentar testes unitários desde o início.

**ADRs relacionados:** ADR-0008 (estratégia de testes), ADR-0007 (estilo com `.clang-format`, mensagens em `stderr`).

**Trecho do TDD §6 que este change altera** (copiado e ajustado ao estado atual):

| Camada | Ferramenta no TDD | Ferramenta neste change |
|---|---|---|
| Unitário | "Executáveis C simples em `tests/unit/` com `assert` (sem framework)" | Executáveis em `tests/unit/` usando **Unity** |
| Integração | `tests/run.sh` (POSIX sh) | Sem mudança. Fica para o change seguinte |

O TDD está congelado, então a troca de "`assert` puro" por uma biblioteca fica registrada no ADR-0008.

**Estado atual relevante:**

- `tests/` existe e está vazio. `app/tests/` e `app/bin/` também existem e estão vazios.
- `Makefile`: lista explícita de fontes em `SRC`, `CC = clang`, `CFLAGS = -Wall -Werror -Wextra -g`, objetos em `app/obj/`. `main.c` está na mesma lista que o resto, então não dá para linkar os testes com `$(OBJS)` sem conflito de `main`.
- Funções públicas testáveis hoje: `parse_args` (`parser.c`), `dispatch_command` (`dispatcher.c`), `fs_create_dir`, `fs_create_file`, `fs_read_file` (`fs/`). As funções de template em `cmd_new.c` são `static`.
- `command.h` **define** o array `g_commands` (defeito conhecido, ADR-0001). Só `dispatcher.c` o inclui, então os testes não podem incluir `command.h`.
- `dispatcher.o` depende de `cmd_new.o` (via `g_commands`), que depende de `fs/*.o`. Os testes linkam todos os objetos da aplicação, menos o `main.o`.

## Goals / Non-Goals

**Goals:**

- Escrever um teste novo exige só criar `tests/unit/test_<módulo>.c`, sem mexer no `Makefile`.
- `make test` funciona num clone limpo, em Linux e macOS, sem instalar nada.
- Um teste que falha ou trava não esconde o resultado dos outros arquivos de teste.
- Os testes compilam com as mesmas flags de aviso da aplicação (`-Wall -Wextra -Werror`).

**Non-Goals:**

- Mocks de funções da libc (`malloc`, `fopen`). Os testes de `fs` usam o sistema de arquivos real num diretório temporário.
- Medir cobertura de código.
- Rodar os testes com sanitizers por padrão. O `Makefile` aceita `CFLAGS` extras, e o alvo `sanitize` chega com o CI (passo 12).

## Decisions

### 1. Biblioteca: Unity

Critérios do pedido: **acessível** (sem instalação, sem `sudo`, Linux e macOS), **fácil de usar** e **flexível** para um projeto com Makefile simples e módulos em `app/src/<módulo>/`.

| Biblioteca | Instalação | Facilidade | Flexibilidade com o nosso Makefile | Observações |
|---|---|---|---|---|
| **Unity** | Nenhuma: 3 arquivos copiados (`unity.c`, `unity.h`, `unity_internals.h`) | Asserts legíveis e específicos (`TEST_ASSERT_EQUAL_STRING`, `TEST_ASSERT_NULL`), `setUp`/`tearDown` | Alta: compila como mais um `.c`, sem gerador obrigatório | Pura C, MIT, a mais usada das quatro, releases ativas (2.6.1, 2.7.0). Exige listar os testes no `main` com `RUN_TEST` |
| Criterion | Pacote do sistema (`libcriterion-dev`, `brew install criterion`) ou build com meson | A melhor: testes se registram sozinhos, sem `main`, cada teste num processo | Média: depende da lib instalada em toda máquina e no CI | Sem `sudo` (máquinas da 42), instalar vira uma barreira |
| cmocka | Pacote do sistema ou build com CMake | Boa, com mocks embutidos | Média: mesma questão de instalação | Ponto forte são os mocks, que não estão nos objetivos |
| acutest | Nenhuma: 1 header | Simples, cada teste num processo | Alta | Asserts genéricos (`TEST_CHECK(strcmp(...) == 0)`), mensagens de falha pobres, sem push desde 2024 |
| `assert` puro (proposta original do TDD) | Nenhuma | Simples, mas para no primeiro erro e não diz o valor esperado | Alta | Sem relatório, sem `setUp`/`tearDown` |

**Escolha: Unity.** É a única que junta as duas qualidades que mais pesam aqui: não precisa instalar nada (atende "acessível" e simplifica o CI em Linux e macOS) e tem asserts que mostram esperado × obtido (atende "fácil"). O Criterion ganha em ergonomia, mas exigir uma lib instalada em cada máquina contraria o primeiro critério. O acutest é igualmente portátil, mas as mensagens de falha genéricas pesam mais no dia a dia do que o isolamento por processo.

**Custo aceito:** cada arquivo de teste tem um `main` com uma linha `RUN_TEST(nome)` por teste. Esquecer essa linha faz o teste não rodar. A mitigação está em Risks.

**Versão:** Unity **v2.6.1**, a última da série 2.6, com a tag registrada em `tests/vendor/unity/VERSION` junto com o `LICENSE.txt`. A v2.7.0 já foi publicada; atualizar é um change futuro que troca os três arquivos.

### 2. Layout de diretórios

```
tests/
  vendor/unity/        unity.c, unity.h, unity_internals.h, LICENSE.txt, VERSION
  unit/
    test_parser.c      parse_args
    test_dispatcher.c  dispatch_command
    test_fs.c          fs_create_dir, fs_create_file, fs_read_file
    support/
      tmpdir.c, tmpdir.h   cria, remove (mkdtemp + remoção recursiva) e monta caminhos (tmpdir_path)
  obj/                 (reservado: não é gerado hoje, mas fclean e .gitignore já o cobrem)
  bin/                 (gerado, ignorado pelo git)
```

- **`tests/unit/`, e não `app/tests/`:** é o caminho previsto no TDD §6 e no ADR-0008, e mantém `app/` só com código da aplicação. Os diretórios vazios `app/tests/` e `app/bin/` ficam como estão; a remoção é assunto do passo 11 (legado).
- **Um executável por arquivo de teste:** se `test_fs` travar com um segfault, `test_parser` ainda roda e reporta. Isso compensa a falta de isolamento por processo da Unity.
- **`support/`:** código auxiliar compartilhado, compilado e linkado em todos os executáveis de teste. A Unity só traz asserts; diretório temporário é responsabilidade nossa.

### 3. Integração com o Makefile

```make
APP_OBJS   = $(filter-out $(OBJ_DIR)/main.o,$(OBJS))

TEST_DIR   = tests
UNITY_DIR  = $(TEST_DIR)/vendor/unity
TEST_SRC   = $(wildcard $(TEST_DIR)/unit/test_*.c)
TEST_SUP   = $(wildcard $(TEST_DIR)/unit/support/*.c)
TEST_BINS  = $(TEST_SRC:$(TEST_DIR)/unit/%.c=$(TEST_DIR)/bin/%)
TEST_FLAGS = $(INCLUDE) -I $(UNITY_DIR) -I $(TEST_DIR)/unit/support

$(TEST_DIR)/bin/%: $(TEST_DIR)/unit/%.c $(TEST_SUP) $(UNITY_DIR)/unity.c $(APP_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(TEST_FLAGS) $^ -o $@

test-unit: $(TEST_BINS)
	@fail=0; for t in $(TEST_BINS); do ./$$t || fail=1; done; exit $$fail

test: test-unit
```

- **`wildcard` nos testes, lista explícita na aplicação:** sem Norminette (ADR-0007), não há regra contra `wildcard`. Ele cumpre o objetivo "teste novo sem mexer no Makefile". A lista explícita do `SRC` da aplicação continua como está, porque mudá-la é escopo do Makefile novo (passos 9 e 12).
- **`APP_OBJS` sem `main.o`:** o `main` vem do arquivo de teste. Linkar todos os objetos (e não só os do módulo testado) evita manter uma lista de dependências por teste. O custo é tempo de link desprezível.
- **O laço roda todos os executáveis e só depois falha:** um arquivo com falha não impede o relatório dos outros.
- **`test` depende de `test-unit`:** quando os testes de integração chegarem, `test` passa a depender também de `test-integration`, sem mudar o uso.
- **Compilação direta (`$^`)**, sem objetos intermediários de teste: são poucos arquivos pequenos. Se o tempo de build incomodar, dá para passar a gerar `tests/obj/` depois, sem mudar os testes.
- **`fclean`** remove `tests/bin/` e `tests/obj/`. O `.gitignore` ganha as duas pastas.
- **Flags:** a Unity compila com `-Wall -Wextra -Werror`. Se algum aviso aparecer com o `clang` local, ele é desligado só para `unity.c` com uma flag específica, e não relaxado para o resto.

### 4. O que os primeiros testes cobrem

Os testes registram o comportamento **atual**, inclusive o que vai mudar, para que cada change seguinte troque a expectativa de forma explícita:

| Arquivo | Casos |
|---|---|
| `test_parser.c` | 4 argumentos preenchem `command`, `type`, `project_name` e zeram `verbose`; `argc` 1, 2, 3 e 5 retornam `1` (comportamento que o passo 3b vai mudar); `ft --help` retorna `1` (regressão D1, que o passo 3a corrige) |
| `test_dispatcher.c` | comando desconhecido retorna `1`. O caminho do comando `new` não é exercitado aqui, porque cria arquivos no diretório corrente; ele é coberto pelos testes de integração |
| `test_fs.c` | `fs_create_dir` cria o diretório e retorna `-1` se ele já existe; `fs_create_file` cria arquivo vazio e trunca um existente; `fs_read_file` lê o conteúdo exato, lê arquivo vazio como `""` e retorna `NULL` para arquivo inexistente |

Os testes de `fs` rodam dentro de um diretório criado por `mkdtemp` no `setUp` e removido no `tearDown`, sem tocar no repositório.

**Saída em `stdout`/`stderr` durante os testes:** `dispatch_command` e `fs_create_dir` imprimem mensagens de erro. Os testes não verificam essas mensagens (o formato muda no passo 2, ADR-0007), só os valores de retorno. O ruído na saída do `make test` é aceito por enquanto.

### 5. Estilo

Os arquivos de teste e de `support/` seguem o `.clang-format`. Os arquivos da Unity em `vendor/` ficam como vieram (não são formatados), e um `.clang-format` com `DisableFormat: true` em `tests/vendor/` impede que uma formatação em massa os altere.

## Risks / Trade-offs

- [Esquecer o `RUN_TEST` de um teste novo, que então nunca roda] → Convenção documentada no README: todo `void test_*` precisa de um `RUN_TEST`. Com `-Wall`, uma função `static` não usada gera aviso e o `-Werror` quebra o build, então os testes são declarados `static`.
- [Segfault num teste interrompe o executável inteiro] → Um executável por arquivo limita o estrago ao arquivo, e o laço do `make test-unit` continua nos demais.
- [Testes de `fs` dependem do sistema de arquivos real] → `mkdtemp` em `/tmp` (ou `$TMPDIR` no macOS), um diretório por teste, removido no `tearDown`.
- [Testes que registram comportamento atual vão quebrar nos passos 3b e 5] → Esperado e desejável: o change que muda o comportamento atualiza o teste, o que deixa a mudança explícita na revisão.
- [Código de terceiros no repositório] → Unity é MIT, com `LICENSE.txt` e `VERSION` ao lado do código. Atualizar é trocar três arquivos.
- [`wildcard` pega arquivos `test_*.c` abandonados] → Aceito. Um arquivo de teste quebrado aparece imediatamente no `make test`.
- [`clang-format` não instalado localmente] → `pipx run clang-format --dry-run --Werror ...`, sem instalar no sistema.

## Migration Plan

Não há migração: o change só adiciona arquivos de teste e alvos no `Makefile`. `make`, `make install` e o binário `ft` não mudam. Rollback é reverter o commit.

## Open Questions

- Habilitar `UNITY_USE_COMMAND_LINE_ARGS` para rodar um teste específico por nome (`./tests/bin/test_fs -n test_read_missing`). Pode ser decidido quando a suíte crescer, sem mudar a estrutura.

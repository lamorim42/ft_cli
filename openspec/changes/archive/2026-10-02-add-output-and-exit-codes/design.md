# Design

## Context

Ver `proposal.md` (Why) para a motivação e `specs/cli/spec.md` e `specs/project-generation/spec.md` para o comportamento exigido.

**Passo do TDD:** este change implementa o **passo 2** do `docs/TDD.md` §7:

| # | Passo | Resolve | Testes que entram |
|---|---|---|---|
| 2 | `output.c`, `ft.h` (códigos de saída) e erros para `stderr` | D5 parcial | A5 |

**ADRs relacionados:** ADR-0007 (mensagens em `stderr`, português, códigos `0`–`3`; este change cobre os itens 1, 2 e 3 da decisão, mas não o 4 e o 5, de estilo e includes, por isso o ADR continua Proposto), ADR-0008 (os testes funcionais e unitários que este change atualiza).

**Trecho do TDD §4.1 que este change implementa** (copiado e ajustado ao estado atual):

```c
// ft.h
enum e_ft_status {
    FT_OK        = 0,
    FT_EUSAGE    = 1,
    FT_EIO       = 2,
    FT_EINTERNAL = 3,
};

// output.h
void ft_err(const char *fmt, ...);    // stderr, prefixo "ft: "
void ft_warn(const char *fmt, ...);   // stderr, prefixo "ft: aviso: "
void ft_info(const char *fmt, ...);   // stdout
void ft_debug(const char *fmt, ...);  // stderr, só se g_verbose
extern int g_verbose;
```

- "Todo o código usa só essas funções para escrever: nenhum `printf` ou `perror` solto fora de `output.c`." Ajuste: vale para o código ativo. O legado (`cli/*`, `core/*`) não é chamado e sai no passo 11; a mensagem de uso do `main.c` (`FALLBACK`) fica como está até o passo 3 (decisão 4).
- "Erros de sistema são formatados com `strerror(errno)` pelo chamador: `ft_err("%s: %s", path, strerror(errno))`."
- "As mensagens são em português (ADR-0007). Centralizá-las mantém o texto num só arquivo." Ajuste: as mensagens ficam nos chamadores, que são poucos; um catálogo de mensagens não compensa ainda.

E do TDD §4.9: "Todas as funções [`fs_*`] retornam `0` ou `-1` com `errno` preservado. Nenhuma imprime mensagens: quem decide o que mostrar é o chamador." Este change aplica essa regra às `fs_*` atuais, sem reescrevê-las (o resto da §4.9 é o passo 5).

**Estado atual relevante:**

| Onde | Hoje | Destino |
|---|---|---|
| `main.c` | `parse_args` falha → `dprintf(STDERR_FILENO, FALLBACK)`, retorna `1` | igual, com `FT_EUSAGE` |
| `dispatcher.c` | `printf("Comando desconhecido: %s\n")` em `stdout`, retorna `1` | `ft_err("comando desconhecido: %s")`, `FT_EUSAGE` |
| `fs_create_dir.c` | `perror("Error creating directory")`, retorna `-1` | só retorna `-1` |
| `fs_create_file.c` | `perror("Error creating file")`, retorna `-1` | só retorna `-1` (função sem uso no `cmd_new`, mas testada) |
| `cmd_new.c` `cmd_new_execute` | `printf("Executando comando NEW...")`; `readlink` falha → `fprintf(stderr)`, `1`; `str_to_upper` `NULL` → `1` sem mensagem; retorno de `fs_create_dir` ignorado; `printf("Estrutura criada...")` | sem a linha inicial; códigos e mensagens da tabela da decisão 3; `ft_info` final |
| `cmd_new.c` `process_template` | `fs_read_file` `NULL` → `fprintf(stderr)`, `1`; `replace_placeholder` `NULL` não checado (D5); `fopen` falha → `fprintf(stderr)`, `1`; `printf("Criado arquivo: ...")` | códigos e mensagens da decisão 3; `ft_info("criado  %s")` |

`cmd_new_execute` é chamado por `dispatch_command`, e `main` retorna o que o dispatcher retorna, então o código de saída do processo é o retorno do comando.

## Goals / Non-Goals

**Goals:**

- Nenhuma chamada a `printf`, `fprintf`, `perror` ou `puts` no código ativo fora de `output.c`, exceto o `FALLBACK` do `main.c`.
- Toda saída de erro do `ft new project` e do dispatcher tem um código da tabela do ADR-0007.
- `output.c` não depende de nenhum outro módulo do projeto (TDD §3.1).

**Non-Goals:**

- Reestruturar `cmd_new.c` (resolução de templates, buffers fixos e render são os passos 5, 6 e 9).
- Ligar o `--verbose` (passo 3).
- Traduzir a mensagem de uso.

## Decisions

### 1. Assinatura e formato das funções de saída

```c
// output.h
#define FT_PRINTF(a, b) __attribute__((format(printf, a, b)))

extern int g_verbose;

void ft_err(const char *fmt, ...) FT_PRINTF(1, 2);   // stderr: "ft: " + msg + "\n"
void ft_warn(const char *fmt, ...) FT_PRINTF(1, 2);  // stderr: "ft: aviso: " + msg + "\n"
void ft_info(const char *fmt, ...) FT_PRINTF(1, 2);  // stdout: msg + "\n"
void ft_debug(const char *fmt, ...) FT_PRINTF(1, 2); // stderr: "ft: debug: " + msg + "\n", só se g_verbose
```

- **A função acrescenta o `\n`**, e o chamador não o escreve. Assim nenhuma mensagem sai sem quebra de linha ou com quebra dupla.
- **`format(printf, ...)`** faz o `-Wall -Werror` acusar formato e argumento incompatíveis. Funciona em `clang` e `gcc`, os dois compiladores suportados.
- **`ft_debug` usa o prefixo `ft: debug: `**: o TDD não define o prefixo; este evita confundir depuração com erro. `g_verbose` é definido em `output.c` com valor `0` e ninguém o liga até o passo 3.
- **Implementação:** `vfprintf` no fluxo certo seguido de `fputc('\n')`. Antes de escrever em `stderr`, `ft_err`, `ft_warn` e `ft_debug` fazem `fflush(stdout)`, para que, num terminal ou num arquivo compartilhado, as linhas `criado` já impressas apareçam antes do erro.
- **`errno`:** as funções salvam e restauram `errno`, para que um chamador possa emitir a mensagem e ainda decidir com base no `errno`.

Alternativa considerada: uma função única `ft_print(level, fmt, ...)`. Rejeitada porque as quatro funções do TDD deixam o nível explícito em cada chamada e são o que os próximos passos esperam.

### 2. Códigos de saída em `ft.h`

O enum do TDD vai para `app/include/ft.h`, que por enquanto só tem isso (`FT_VERSION` chega com o comando `version`, no passo 3). `main.c`, `dispatcher.c` e `cmd_new.c` passam a retornar `FT_*` em vez de `0`/`1` literais.

### 3. Mensagens e códigos do `ft new project`

| Situação | Mensagem (`ft_err`, já com o prefixo `ft: `) | Código |
|---|---|---|
| `readlink("/proc/self/exe")` falha | `não foi possível localizar os templates: <strerror>` | `FT_EIO` |
| `str_to_upper` retorna `NULL` | `memória insuficiente` | `FT_EINTERNAL` |
| `fs_create_dir` falha com `errno != EEXIST` | `não foi possível criar o diretório <caminho>: <strerror>` | `FT_EIO` |
| `fs_create_dir` falha com `EEXIST` | nenhuma: segue a geração | — |
| `fs_read_file` retorna `NULL` | `não foi possível ler o template <caminho>: <strerror>` | `FT_EIO` |
| `replace_placeholder` retorna `NULL` | `memória insuficiente` | `FT_EINTERNAL` |
| `fopen(saída, "w")` falha | `não foi possível criar o arquivo <caminho>: <strerror>` | `FT_EIO` |
| Sucesso, por arquivo | `ft_info("criado  %s", caminho)` | — |
| Sucesso, no fim | `ft_info("Projeto '%s' criado com o template 'default'.", nome)` | `FT_OK` |

- **`readlink` vira `FT_EIO`, não `FT_EINTERNAL`:** o efeito para o usuário é "template não encontrado", que o SPEC §4 classifica como `2`. É o caminho que o macOS segue hoje (D6), e lá o teste funcional confirma a mensagem.
- **`EEXIST` ignorado:** mantém o comportamento atual de gerar de novo sobre um projeto existente (D3), agora sem as nove linhas de `perror`. O passo 8 transforma isso em erro `2`.
- **Qualquer outro erro de diretório para a geração:** hoje o erro é ignorado e a falha aparece depois, ao criar um arquivo, com uma mensagem que não aponta a causa. `ft new project a/b` passa a falhar logo no primeiro `mkdir`, sem criar nada.
- **`strerror` em inglês:** a `ft` não chama `setlocale`, então o motivo vem sempre da locale `C` (`No such file or directory`). A parte em português é a nossa; traduzir mensagens do sistema não está nos objetivos do ADR-0007.
- **`fs_read_file` e `errno`:** quando o `fopen` falha, o `errno` é o do `fopen`. Quando o `malloc` interno falha, o `errno` é `ENOMEM` e a mensagem sai como "não foi possível ler o template ...: Cannot allocate memory", com código `2` e não `3`. Aceito até o `fs_read_file` novo (passo 5), que distingue os casos.
- **Mensagem `criado` impressa logo depois de gravar cada arquivo**, como hoje. O SPEC pede que elas sejam acumuladas até o `rename` atômico, o que é o passo 8.
- **`fputs`/`fclose` sem checagem:** fica como está; a escrita robusta é o `fs_write_file` do passo 5.

Para propagar os códigos, `create_project_dir`, `process_template` e `process_templates` passam a retornar `t_ft_status` (`int`) em vez de `void`/`0`/`1`, e `cmd_new_execute` retorna o primeiro código diferente de `FT_OK`. Os buffers fixos e a estrutura das funções não mudam.

### 4. Mensagem de uso no `main.c`

O `dprintf(STDERR_FILENO, FALLBACK)` continua. Ele já vai para `stderr` com código `1`, e o texto (em inglês, sem prefixo `ft: `) é substituído pelo uso do comando `help` no passo 3. Passá-lo por `ft_err` agora acrescentaria `ft: ` a um texto de várias linhas que vai ser trocado. A spec `cli` registra a exceção.

### 5. Testes

**Unitários** (Unity):

- **`tests/unit/support/capture.{c,h}`:** redireciona `stdout` ou `stderr` para um arquivo temporário (`dup`/`dup2`), roda o código, restaura o descritor e devolve o texto capturado. Usado por `test_output.c`, `test_dispatcher.c` e `test_fs.c`.
- **`test_output.c`:** `ft_err` escreve `ft: x 42\n` em `stderr` e nada em `stdout`; `ft_warn` usa `ft: aviso: `; `ft_info` escreve em `stdout` sem prefixo; `ft_debug` não escreve nada com `g_verbose = 0` e escreve `ft: debug: ...` com `g_verbose = 1`; `errno` é o mesmo antes e depois de `ft_err`.
- **`test_dispatcher.c`:** comando desconhecido retorna `FT_EUSAGE`, `stderr` igual a `ft: comando desconhecido: foo\n` e `stdout` vazio.
- **`test_fs.c`:** `fs_create_dir` e `fs_create_file` que falham não escrevem nada em `stderr`, e `fs_create_dir` sobre diretório existente deixa `errno == EEXIST`.

**Funcionais** (shell):

| Arquivo | Mudança |
|---|---|
| `test_new_project.sh` | `stdout` do A6 passa a ser o da spec `project-generation` |
| `test_usage.sh` | `ft foo a b`: `stdout` vazio, `stderr` = `ft: comando desconhecido: foo`, código `1`. `ft` e `ft --help` sem mudança |
| `test_known_defects.sh` | repetir A6: código `0` e agora também `stderr` vazio |
| `test_errors.sh` (novo) | `ft new project a/b` → mensagem da spec, código `2`, `a` não existe. Template ilegível: copia `ft` e `ft_templates/` para `inst/`, faz `chmod 000 inst/ft_templates/default/main.tpl`, roda `inst/ft new project foo` → código `2`, mensagem da spec, sem `criado  foo/app/src/main.c` (pulado se `id -u` for `0`, porque root ignora a permissão). Fora do Linux: `ft new project foo` → `ft: não foi possível localizar os templates: ...`, código `2`, `foo` não existe |

O caso de falta de memória (`FT_EINTERNAL`) não tem teste automatizado: forçar o `malloc` a falhar exigiria interposição de símbolos, que não é portável entre Linux e macOS. Ele é verificado por revisão de código (tarefas).

### 6. Build

`app/src/output/output.c` entra na lista `SRC` do `Makefile`. Os testes unitários já linkam todos os objetos menos o `main.o` (`APP_OBJS`), então enxergam `output.o` sem mudança na regra de testes. `capture.c` entra sozinho pelo `wildcard` de `tests/unit/support/`.

## Risks / Trade-offs

- [Mudança de texto em `stdout` quebra quem lia a saída antiga] → É a primeira versão com formato definido; o SPEC §5.3 é o contrato daqui em diante. Marcado como **BREAKING** na proposta.
- [`stdout` e `stderr` intercalados fora de ordem quando os dois vão para o mesmo destino] → `fflush(stdout)` antes de cada escrita em `stderr` (decisão 1).
- [Falta de memória sem teste automatizado] → Revisão das três checagens em tarefa própria; o `tpl_render` do passo 6 nasce com testes unitários de `NULL`.
- [Mensagem de `ENOMEM` em `fs_read_file` sai com código `2`] → Aceito e documentado (decisão 3) até o passo 5.
- [Exceção do `FALLBACK` contraria "nenhum printf fora de `output.c`"] → Uma única exceção, registrada na spec e no design, removida no passo 3.

## Migration Plan

Sem migração de dados. O comportamento visível muda (mensagens e códigos de saída), e os testes funcionais registram o novo contrato. Rollback é reverter o commit.

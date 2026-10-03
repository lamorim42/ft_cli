# TDD: ft_cli (Technical Design Document)

- **Versão do documento:** 0.1 (rascunho)
- **Data:** 2026-10-01
- **Status:** Congelado em 2026-10-02. A fonte de verdade agora é o OpenSpec (`openspec/specs/`); este documento fornece o design de cada change (o §7 é o roteiro). Não edite.
- **Base:** branch `feature/refatorando`, commit `85bce6c`
- **Relacionados:** [SPEC](SPEC.md), [ADRs](adr/README.md)

O [SPEC](SPEC.md) define o comportamento esperado da `ft`. Este documento define como implementá-lo: módulos, interfaces, algoritmos, tratamento de erros, build e a ordem de migração a partir do código atual.

---

## 1. Contexto

### 1.1 Objetivo

Transformar a `ft` numa CLI modular e testável, em que:

- um comando novo custe um arquivo `cmd_*.c` e uma linha numa tabela;
- um template novo não exija recompilar;
- nenhuma execução apague o trabalho do usuário ou escreva fora do diretório esperado.

### 1.2 Fora do escopo

- Templates com lógica (condicionais, laços). Só há substituição de `{{CHAVE}}`.
- Suporte a Windows.
- Atualizar projetos já gerados quando o template mudar.

---

## 2. Estado atual

### 2.1 Fluxo

```
main ─► parse_args (exige argc == 4) ─► dispatch_command ─► cmd_new_execute
                                                              ├─ readlink("/proc/self/exe")
                                                              ├─ fs_create_dir × 9 (o retorno é ignorado)
                                                              └─ process_templates
                                                                   └─ process_template × 4
                                                                        ├─ fs_read_file
                                                                        ├─ replace_placeholder × 2
                                                                        └─ fopen("w") + fputs
```

### 2.2 Inventário

| Área | Arquivos | Situação |
|---|---|---|
| Entrada | `main.c` | Fluxo novo. O `main` antigo continua no arquivo, comentado. |
| Parser | `parser/parser.c` | Aridade fixa (4) e `verbose` sempre `0` |
| Dispatcher | `dispatcher/dispatcher.c` | Busca linear em `g_commands`. O erro vai para `stdout`. |
| Registro | `include/command.h` | **Define** o array no header |
| Comando `new` | `commands/cmd_new.c` (~200 linhas) | Mistura resolução de caminho, render, mkdir e escrita. Ignora `ctx->type`. |
| FS | `fs/fs_create_dir.c`, `fs_create_file.c`, `fs_read_file.c` | Retornos de `ftell`/`fread` não checados. `fs_create_file` não é usado. |
| Legado | `cli/*`, `core/*`, `utils/cli_utils.c`, as strings de `files_contents.h` | Compilado, mas sem uso |
| Templates | `ft_templates/default/{include,main,makefile,readme}.tpl` | Falta o `gitignore.tpl` |
| Build | `Makefile` | `.PHONY=` em vez de `.PHONY:`. O `install` copia os templates para `/usr/local/bin`. Sem `-MMD`. |
| Testes | `tests/`, `app/tests/` | Diretórios vazios |
| CI | `.github/workflows/pr-to-*.yml` | Só abrem PRs, e usam `github.head_ref`, que é vazio em `push` |

### 2.3 Defeitos que este design corrige

| ID | Defeito | Onde é tratado |
|---|---|---|
| D1 | `ft --help` cai no `FALLBACK` | §4.2, §4.3 |
| D2 | O tipo e o nome não são validados, e nomes com `/` ou `..` escapam do diretório | §4.4 |
| D3 | Um projeto existente é sobrescrito sem aviso | §4.8 |
| D4 | O `.gitignore` não é gerado | §4.7, template `default` |
| D5 | `NULL` não checado em `replace_placeholder`, `ftell` e `fread` | §4.5, §4.9 |
| D6 | `/proc/self/exe` só funciona no Linux, e o buffer de 512 bytes pode truncar | §4.6 |
| D7 | `g_commands` definido no header | §4.3 |
| D8 | Buffers fixos de 512/256 bytes para caminhos | §4.9 (caminhos dinâmicos) |

---

## 3. Arquitetura

### 3.1 Módulos e dependências

```
                 ┌──────────┐
                 │  main.c  │
                 └────┬─────┘
          ┌───────────┼────────────┐
          ▼           ▼            ▼
      ┌────────┐ ┌──────────┐ ┌─────────┐
      │ parser │ │dispatcher│ │ output  │◄──────────────┐
      └────────┘ └────┬─────┘ └─────────┘               │
                      ▼                                 │
                ┌───────────┐  registry.c               │
                │ commands/ │  cmd_help, cmd_version,   │
                │           │  cmd_new, cmd_list        │
                └─────┬─────┘                           │
          ┌───────────┼──────────────┐                  │
          ▼           ▼              ▼                  │
     ┌─────────┐ ┌──────────┐ ┌────────────┐            │
     │validate │ │ template │ │ generator  │────────────┤
     └─────────┘ │ resolve  │ │            │            │
                 │ manifest │ └─────┬──────┘            │
                 │ render   │       │                   │
                 └────┬─────┘       │                   │
                      └──────┬──────┘                   │
                             ▼                          │
                         ┌──────┐   ┌───────┐           │
                         │  fs  │──►│ strbuf│───────────┘
                         └──────┘   └───────┘
```

**Regras de dependência:**

- As setas apontam só para baixo, e não há ciclos.
- `fs`, `strbuf` e `output` não dependem de nenhum módulo do projeto.
- `template/*` não conhece comandos nem o `t_cli_context`.
- Só `commands/*` conhece o `t_cli_context`.

### 3.2 Árvore de código alvo

```
app/
├── include/
│   ├── ft.h            # códigos de saída, FT_VERSION
│   ├── cli.h           # t_cli_context, parse_args
│   ├── command.h       # t_command, extern g_commands[], protótipos cmd_*
│   ├── output.h        # ft_err, ft_warn, ft_info, ft_debug
│   ├── validate.h
│   ├── template.h      # t_tpl_var, tpl_render, tpl_resolve, manifest
│   ├── generator.h
│   ├── fs.h
│   └── strbuf.h
└── src/
    ├── main.c
    ├── cli/parser.c
    ├── dispatcher/dispatcher.c
    ├── commands/{registry,cmd_help,cmd_version,cmd_new,cmd_list}.c
    ├── output/output.c
    ├── validate/validate_identifier.c
    ├── template/{template_render,template_resolve,manifest}.c
    ├── generator/generator.c
    ├── fs/{fs_exists,fs_mkdir_p,fs_read_file,fs_write_file,fs_path,fs_rm_rf}.c
    └── utils/strbuf.c
```

---

## 4. Design detalhado

### 4.1 Códigos de saída e saída de texto

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

- Todo o código usa só essas funções para escrever: nenhum `printf` ou `perror` solto fora de `output.c`.
- Erros de sistema são formatados com `strerror(errno)` pelo chamador: `ft_err("%s: %s", path, strerror(errno))`.
- As mensagens são em português (ADR-0007). Centralizá-las mantém o texto num só arquivo.

### 4.2 Parser (ADR-0002)

```c
// cli.h
typedef struct s_cli_context {
    const char  *command;   // NULL se não houver
    int          argc;      // argumentos posicionais depois do comando
    char       **argv;      // apontam para o argv original
    const char  *template;  // --template, padrão "default"
    bool         force;
    bool         git;
    bool         help;
    bool         verbose;
} t_cli_context;

int parse_args(int argc, char **argv, t_cli_context *ctx);  // FT_OK ou FT_EUSAGE
```

**Algoritmo:** uma passada sobre `argv[1..]`.

1. Depois de um `--`, todo token é posicional.
2. Um token que começa com `--` é uma flag longa. Separa-se no `=` se houver; senão, se a flag exige valor, consome-se o próximo token.
3. Um token que começa com `-` e tem tamanho 2 é uma flag curta.
4. Qualquer outro token é posicional, e é compactado no próprio `argv` (in-place, sem alocação).
5. O primeiro posicional vira `ctx->command`, e os demais ficam em `ctx->argv`/`ctx->argc`.
6. Uma flag desconhecida gera `ft_err("opção desconhecida: %s")` e retorna `FT_EUSAGE`.

**Normalização:**

- `--help` sem comando vira `command = "help"`.
- `--help` com comando vira `command = "help"` e `argv = [comando]`.
- `--version` vira `command = "version"`.

**Por que não `getopt_long`:** ele funciona, mas a permutação de argumentos do GNU difere entre a glibc e o macOS, e o parser manual tem menos de 80 linhas. Fica aberto trocar (ADR-0002).

As flags de comando (`--force`, `--git`, `--template`) são aceitas globalmente pelo parser, mas só o comando que as usa as lê. É uma simplificação consciente: com 3 flags, um parser por comando seria excesso.

### 4.3 Registro e dispatcher (ADR-0001)

```c
// command.h
typedef int (*t_command_fn)(t_cli_context *ctx);

typedef struct s_command {
    const char   *name;
    const char   *summary;   // uma linha, usada pelo `ft help`
    const char   *usage;     // texto do `ft help <cmd>`
    t_command_fn  execute;
} t_command;

extern const t_command g_commands[];
const t_command *command_find(const char *name);
```

- `registry.c` define a tabela, que termina em `{0}`.
- `dispatch_command` faz:
  - comando `NULL` → imprime o uso curto em `stderr` e retorna `FT_EUSAGE`;
  - comando não encontrado → `ft_err` e `FT_EUSAGE`;
  - senão → `execute(ctx)`.
- O `cmd_help` percorre a mesma tabela, então a ajuda nunca fica desatualizada em relação aos comandos.
- Os comandos com subtipo (`new project`, `new module`, `list templates`) recebem o subtipo em `ctx->argv[0]` e despacham internamente com uma cadeia simples de `strcmp`.

### 4.4 Validação (ADR-0006)

```c
// validate.h
typedef enum e_name_err {
    NAME_OK, NAME_EMPTY, NAME_TOO_LONG, NAME_BAD_START, NAME_BAD_CHAR, NAME_RESERVED
} t_name_err;

t_name_err validate_identifier(const char *name);
const char *validate_strerror(t_name_err e);
char       *validate_suggest(const char *name);  // troca [^A-Za-z0-9_] por '_'; prefixa '_' se começar com dígito
```

- As palavras reservadas ficam numa tabela estática com as 44 palavras-chave do C11, mais `main`.
- O nome é validado **antes** de resolver o template e antes de qualquer acesso ao disco.
- Como o nome só aceita `[A-Za-z0-9_]`, `/` e `..` ficam impossíveis por construção.

### 4.5 Motor de templates (ADR-0004)

```c
// template.h
typedef struct s_tpl_var { const char *key; const char *value; } t_tpl_var;

// vars termina em {NULL, NULL}. Retorna uma string alocada ou NULL (ENOMEM).
// Se unknown != NULL, recebe o número de chaves desconhecidas encontradas.
char *tpl_render(const char *src, const t_tpl_var *vars, size_t *unknown);
```

**Algoritmo:** uma passada só, em O(n·k), onde k é o número de variáveis (pequeno).

```
buf = strbuf vazio
p = src
enquanto houver "{{" a partir de p, na posição o:
    anexa src[p..o) a buf
    procura "}}" a partir de o+2, na posição c
    se não achar: anexa o resto e termina
    chave = src[o+2..c)
    se a chave casa [A-Z0-9_]+ e existe em vars: anexa o valor
    senão: anexa src[o..c+2) literalmente e incrementa unknown
    p = c + 2
anexa o resto
retorna strbuf_detach(buf)
```

- O valor substituído nunca é processado de novo, o que impede recursão e injeção de placeholders.
- `strbuf` é um buffer dinâmico (`char *data; size_t len, cap;`) que dobra de capacidade. Uma falha de `realloc` faz o render inteiro retornar `NULL`.
- Este algoritmo substitui `replace_placeholder`, que fazia uma passada por variável e alocava a cada uma.

**Variáveis montadas pelo `cmd_new`:** `PROJECT_NAME`, `PROJECT_NAME_UPPER`, `INCLUDE_NAME` (`<UPPER>_H`), `FT_VERSION` e `YEAR` (via `localtime`).

### 4.6 Resolução de templates (ADR-0003)

```c
typedef struct s_tpl_source {
    char *dir;          // ex.: /usr/local/share/ft_cli/templates/default
    char *origin;       // diretório base de onde veio
} t_tpl_source;

int  tpl_resolve(const char *name, t_tpl_source *out);           // FT_OK / FT_EIO
int  tpl_list(void (*cb)(const char *name, const char *origin, void *ud), void *ud);
void tpl_source_free(t_tpl_source *s);
```

**Bases de busca, nesta ordem:**

1. `getenv("FT_TEMPLATE_DIR")`;
2. `$XDG_CONFIG_HOME/ft_cli/templates`, ou `$HOME/.config/ft_cli/templates`;
3. a constante `FT_TEMPLATE_DIR`, definida na compilação.

**Como a busca funciona:**

- O `tpl_resolve` procura `<base>/<name>/manifest` em cada base e retorna a primeira encontrada. A precedência é **por template**, não por diretório: um usuário com só um template personalizado continua vendo os templates instalados. Isto refina a redação do ADR-0003.
- O `tpl_list` percorre as bases com `opendir`/`readdir` e usa um conjunto simples (array de nomes já vistos) para respeitar a precedência.
- O `<name>` do `--template` passa pela mesma regra de identificador da §4.4, o que impede `--template=../../etc`.
- Em caso de erro, a mensagem lista as bases procuradas, cada uma numa linha.
- **Execução sem instalar:** o `Makefile` passa `-DFT_TEMPLATE_DIR` apontando para `$(CURDIR)/ft_templates` nos alvos de desenvolvimento e teste. Assim, `./ft` funciona sem instalar e sem depender de `/proc/self/exe`.

### 4.7 Manifest (ADR-0005)

```c
typedef enum e_mf_kind { MF_DIR, MF_FILE } t_mf_kind;

typedef struct s_mf_entry {
    t_mf_kind  kind;
    char      *src;   // MF_FILE: caminho do .tpl, relativo à pasta do template
    char      *dst;   // já renderizado e relativo à raiz do projeto
    int        line;  // para mensagens de erro
} t_mf_entry;

typedef struct s_manifest {
    t_mf_entry *entries;
    size_t      count;
} t_manifest;

int  manifest_load(const char *tpl_dir, const t_tpl_var *vars, t_manifest *out);
void manifest_free(t_manifest *m);
```

**Parsing:**

1. Lê o arquivo inteiro com `fs_read_file` e percorre linha a linha.
2. Remove o comentário (do `#` em diante) e os espaços das pontas, e separa em campos por espaço ou tab.
3. Valida a diretiva e o número de campos.
4. Renderiza o `dst` com `tpl_render`.
5. Valida o `src` e o `dst` com `path_is_safe_relative()`: não pode ser vazio, não pode começar com `/`, nenhum componente pode ser `..`, e não pode conter `\0`.
6. Para cada `MF_FILE`, confere com `access(src, R_OK)`.

O carregamento é **tudo ou nada**: qualquer erro libera o que já foi alocado e retorna `FT_EIO` com `arquivo:linha: motivo`. Toda a validação acontece antes de o gerador tocar no disco, o que garante os critérios A15 e A16 do SPEC.

### 4.8 Gerador e atomicidade (ADR-0006)

```c
typedef struct s_gen_opts {
    const char *tpl_dir;
    const char *dest;     // ex.: "minishell"
    bool        force;
} t_gen_opts;

int generate(const t_manifest *m, const t_tpl_var *vars, const t_gen_opts *o);
```

**Sem `--force` (caminho padrão):**

1. Se `fs_exists(dest)`, retorna `FT_EIO` com a mensagem "já existe".
2. Cria um diretório temporário ao lado do destino: `mkdtemp("<dest>.ft-XXXXXX")`. Ele fica no mesmo diretório pai, logo no mesmo sistema de arquivos.
3. Aplica todas as entradas dentro do diretório temporário:
   - `MF_DIR` → `fs_mkdir_p`;
   - `MF_FILE` → `fs_mkdir_p(dirname)`, depois `fs_read_file` + `tpl_render` + `fs_write_file`.
4. Se tudo der certo, `rename(tmp, dest)`, que é atômico no POSIX. Se outro processo criou `dest` nesse meio-tempo, o `rename` falha com `EEXIST`/`ENOTEMPTY` e o caso é tratado como "já existe".
5. Em qualquer falha, `fs_rm_rf(tmp)` e retorna o erro.
6. As mensagens "criado <caminho>" são acumuladas durante a geração e só impressas depois do `rename`, já com o caminho final.

**Com `--force`:** escreve direto em `dest`, criando os diretórios que faltarem. Os arquivos fora do manifest não são tocados. Neste modo não há atomicidade, e a ajuda do comando documenta isso.

**`fs_rm_rf`** usa `nftw(..., FTW_DEPTH | FTW_PHYS)`. Só é chamado sobre caminhos criados pelo próprio `mkdtemp`, nunca sobre entrada do usuário.

**`--git`:** depois do sucesso, `fork` + `execlp("git", "git", "init", "-q", dest, NULL)`. Se o `exec` falhar, gera um aviso e a saída continua `0` (conforme o SPEC).

### 4.9 Camada de sistema de arquivos

```c
bool  fs_exists(const char *path);                       // usa lstat, então links também contam
int   fs_mkdir_p(const char *path, mode_t mode);         // EEXIST em diretório não é erro
char *fs_read_file(const char *path, size_t *len);       // checa fopen, fseek, ftell, fread e malloc
int   fs_write_file(const char *path, const char *data, size_t len); // escreve em tmp e faz rename
char *fs_path_join(const char *a, const char *b);        // alocado; sem buffers fixos
int   fs_rm_rf(const char *path);
```

- Todas as funções retornam `0` ou `-1` com `errno` preservado. Nenhuma imprime mensagens: quem decide o que mostrar é o chamador.
- O `fs_write_file` escreve em `<path>.tmp` e depois faz `rename`. Assim, mesmo no modo `--force`, um arquivo nunca fica truncado pela metade.
- O `fs_read_file` é reescrito com `fstat` no lugar do par `fseek`/`ftell` e confere o retorno de `fread` em laço.
- **Modos:** diretórios `0755` e arquivos `0644`, ambos sujeitos ao `umask`.

### 4.10 Fluxo completo: `ft new project minishell`

```
main
 ├─ parse_args                    → ctx{command="new", argv=["project","minishell"], template="default"}
 └─ dispatch_command → cmd_new
      ├─ argv[0] == "project"?            senão FT_EUSAGE
      ├─ validate_identifier(argv[1])     senão FT_EUSAGE + sugestão
      ├─ validate_identifier(template)    senão FT_EUSAGE
      ├─ monta vars[]
      ├─ tpl_resolve("default")           senão FT_EIO (lista as bases)
      ├─ manifest_load(dir, vars)         senão FT_EIO (arquivo:linha)
      ├─ generate(manifest, vars, opts)   senão FT_EIO (ou FT_EINTERNAL em ENOMEM)
      ├─ [--git] git init
      ├─ ft_info("Projeto '%s' criado...")
      └─ libera tudo                      → FT_OK
```

**Disciplina de memória:**

- Cada função que aloca tem seu par `*_free`.
- O `cmd_new` agrupa tudo o que aloca numa struct local e a libera num único `cmd_new_cleanup()`, chamado em todos os retornos. Isso evita `goto`, que a Norminette proíbe.

---

## 5. Build

```make
NAME        = ft
VERSION     = 1.0.0
PREFIX     ?= /usr/local
DATADIR     = $(PREFIX)/share/ft_cli/templates
CC         ?= cc
CFLAGS     += -Wall -Wextra -Werror -std=c99 -D_POSIX_C_SOURCE=200809L
CPPFLAGS   += -I app/include -MMD -MP \
              -DFT_VERSION=\"$(VERSION)\" -DFT_TEMPLATE_DIR=\"$(TPL_DIR)\"
# dev: aponta para a pasta do repositório; o install recompila com DATADIR
TPL_DIR    ?= $(CURDIR)/ft_templates

SRC   := $(shell find app/src -name '*.c')   # ou uma lista explícita, se a Norminette exigir
OBJ   := $(SRC:app/src/%.c=app/obj/%.o)
DEP   := $(OBJ:.o=.d)

app/obj/%.o: app/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

install:
	$(MAKE) fclean
	$(MAKE) TPL_DIR=$(DATADIR) $(NAME)
	install -d $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(DATADIR)
	install -m 755 $(NAME) $(DESTDIR)$(PREFIX)/bin/
	cp -R ft_templates/. $(DESTDIR)$(DATADIR)/

uninstall:
	rm -f  $(DESTDIR)$(PREFIX)/bin/$(NAME)
	rm -rf $(DESTDIR)$(PREFIX)/share/ft_cli

test: $(NAME)
	./tests/run.sh ./$(NAME)

sanitize: CFLAGS += -g -fsanitize=address,undefined -fno-omit-frame-pointer
sanitize: LDFLAGS += -fsanitize=address,undefined
sanitize: re

-include $(DEP)
.PHONY: all clean fclean re install uninstall test sanitize
```

- O `install` recompila com o `TPL_DIR` final, evitando um binário instalado que aponte para a pasta de desenvolvimento.
- `DESTDIR` permite empacotamento e permite testar o `install` no CI sem `sudo`.
- Para seguir à risca a Norminette, troca-se o `find` por uma lista explícita de fontes.

---

## 6. Testes (ADR-0008)

| Camada | Ferramenta | Alvo |
|---|---|---|
| Unitário | Executáveis C simples em `tests/unit/` com `assert` (sem framework) | `tpl_render`, `validate_identifier`, `path_is_safe_relative`, `parse_args`, `manifest_load` |
| Integração | `tests/run.sh` (POSIX sh) | Os critérios A1–A20 do SPEC, cada um num `mktemp -d` próprio |
| Memória | `make sanitize && make test`, e `valgrind --error-exitcode=1` no Linux | A21 |
| Estilo | `norminette` ou `clang-format --dry-run --Werror` | Conforme a decisão do ADR-0007 |

**Para o caso A20** (falha no meio da geração), usa-se um template de teste com um `.tpl` sem permissão de leitura (`chmod 000`). O teste é pulado se rodar como root, já que root ignora a permissão.

**CI** (`.github/workflows/ci.yml`, em `pull_request` e `push`):

- matriz `ubuntu-latest` × `macos-latest`;
- passos: `make` → `make test` → `make sanitize test` → `make install DESTDIR=$PWD/stage` → `make uninstall DESTDIR=...`;
- nos workflows existentes, trocar `github.head_ref` por `github.ref_name`.

---

## 7. Plano de migração

Cada passo é um PR pequeno que mantém o build e o `ft new project` funcionando.

| # | Passo | Resolve | Testes que entram |
|---|---|---|---|
| 1 | `tests/run.sh` com A6 e A7 (snapshot do comportamento atual) | — | A6, A7 |
| 2 | `output.c`, `ft.h` (códigos de saída) e erros para `stderr` | D5 parcial | A5 |
| 3 | `registry.c`, `cmd_help`, `cmd_version` e o parser novo | D1, D7 | A1–A5, A13 |
| 4 | `validate_identifier` e checagem de tipo no `cmd_new` | D2 | A10–A12 |
| 5 | `fs_*` novo (`fs_exists`, `fs_mkdir_p`, `fs_path_join`, `fs_read_file` robusto) | D5, D8 | — |
| 6 | `strbuf` e `tpl_render`, com testes unitários | D5 | unitários |
| 7 | `manifest_load`, `gitignore.tpl` e `manifest` do `default` | D4 | A15, A16 |
| 8 | `generator` com `mkdtemp` + `rename`, e `--force` | D3 | A8, A9, A20 |
| 9 | `tpl_resolve`, `tpl_list`, `cmd_list` e o Makefile novo (`install`/`uninstall`) | D6 | A14, A17, A18 |
| 10 | `--git` | — | A19 |
| 11 | Remoção do legado (`cli/*`, `core/*`, `cli_utils.c`, strings de `files_contents.h`, `main` comentado) | — | todos |
| 12 | Workflow de CI (Linux + macOS, sanitizers) | — | A21 |

Os passos 1 e 3 vêm primeiro de propósito: o teste de snapshot protege tudo o que vem depois, e o `--help` é a regressão mais visível.

---

## 8. Riscos e questões em aberto

| Item | Tipo | Encaminhamento |
|---|---|---|
| Idioma das mensagens | Decidido | Português (ADR-0007). |
| Norminette ou `.clang-format` | Decidido | `.clang-format`, sem Norminette (ADR-0007). |
| `rename` entre sistemas de arquivos diferentes | Risco baixo | O temporário é criado no mesmo diretório pai, então o `rename` nunca cruza sistemas de arquivos. |
| Nomes com hífen rejeitados | Experiência de uso | Mensagem com sugestão (`validate_suggest`) |
| `new module` editando o `Makefile` do usuário | Risco médio | Só insere após o marcador `# ft:sources`. Sem o marcador, não edita e emite um aviso. |
| Precedência por template, não por diretório | Diverge da redação do ADR-0003 | Atualizar o ADR-0003 quando este TDD for aceito |
| Código de saída `3` (`FT_EINTERNAL`) | Acréscimo ao ADR-0007 | Atualizar o ADR-0007 |

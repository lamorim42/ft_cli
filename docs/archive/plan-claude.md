> **Arquivado em 2026-10-02.** O conteúdo deste plano foi incorporado ao [SPEC](../SPEC.md), ao [TDD](../TDD.md) e aos [ADRs](../adr/README.md).

# Plano de melhorias: ft_cli

Análise feita sobre a branch `feature/refatorando` (commit `85bce6c`). Nenhum código foi alterado; este documento só lista as sugestões, ordenadas por prioridade.

---

## 1. Bugs e regressões (prioridade alta)

### 1.1 `ft --help` deixou de funcionar
O `main` novo (`app/src/main.c`) chama `parse_args`, que exige `argc == 4`. Com isso, `ft --help` cai sempre no `FALLBACK`, e a mensagem `HELP` ficou sem uso. A própria mensagem de fallback manda o usuário rodar `ft --help`.

**Sugestão:** registrar `help` como um comando na tabela (`{"help", cmd_help_execute}`) e fazer o parser aceitar `--help`/`-h` convertendo para `ctx->command = "help"`.

### 1.2 A validação de entrada foi perdida
`is_valid_option`, `is_valid_arg` e `is_valid_name` ainda são compilados, mas o novo fluxo não os chama. Hoje:
- `ctx->type` é ignorado: `ft new qualquercoisa foo` cria um projeto.
- O nome do projeto não é validado. Nomes como `meu-projeto` ou `1abc` geram um include guard inválido (`MEU-PROJETO_H`), e nomes com `/` ou `..` criam diretórios fora do lugar esperado.

**Sugestão:** validar o `type` dentro do próprio comando (`cmd_new`) e mover a validação do nome para uma função reutilizável (ex.: `validate_identifier()` em `utils/`), aceitando apenas `[A-Za-z_][A-Za-z0-9_]*`.

### 1.3 Projeto existente é sobrescrito
`fs_create_dir` só imprime um erro quando o diretório já existe, e o fluxo continua. Em seguida, `process_template` abre os arquivos com `fopen(..., "w")` e **sobrescreve** `main.c`, `Makefile` etc. Rodar `ft new project minishell` duas vezes apaga o trabalho do usuário.

**Sugestão:**
- Adicionar `fs_exists()` (já está comentado em `fs.h`) e abortar se o diretório raiz do projeto já existir.
- Opcionalmente, criar uma flag `--force` para permitir sobrescrever.
- Checar o retorno de `fs_create_dir` e interromper em caso de erro.

### 1.4 `.gitignore` não é mais gerado
O README diz que o projeto gerado tem `.gitignore`, mas não existe `gitignore.tpl` em `ft_templates/default/` nem entrada correspondente em `process_templates`.

**Sugestão:** criar `ft_templates/default/gitignore.tpl` (com `app/obj/`, o binário `{{PROJECT_NAME}}` etc.) e adicioná-lo à lista.

### 1.5 Retornos `NULL` não verificados
Em `process_template`, o retorno de `replace_placeholder` (que pode ser `NULL` se o `malloc` falhar) é repassado sem checagem. Em `fs_read_file`, os retornos de `ftell`/`fread` também não são verificados.

---

## 2. Instalação e localização dos templates

Hoje o `make install` copia `ft_templates/` para **dentro de `/usr/local/bin`**, e o `cmd_new` descobre o caminho via `/proc/self/exe`. Problemas:
- `/usr/local/bin` deve conter apenas executáveis.
- `/proc/self/exe` só existe no Linux (não funciona no macOS, onde muita gente da 42 usa).
- `exe_path[512]` pode truncar caminhos longos.

**Sugestão:** definir o diretório de templates em tempo de compilação e permitir sobrescrevê-lo por variável de ambiente.

```make
PREFIX       ?= /usr/local
TEMPLATE_DIR  = $(PREFIX)/share/ft_cli/templates
CFLAGS       += -DFT_TEMPLATE_DIR=\"$(TEMPLATE_DIR)\"

install: $(NAME)
	install -d $(PREFIX)/bin $(TEMPLATE_DIR)
	install -m 755 $(NAME) $(PREFIX)/bin/
	cp -r ft_templates/* $(TEMPLATE_DIR)/

uninstall:
	rm -f $(PREFIX)/bin/$(NAME)
	rm -rf $(PREFIX)/share/ft_cli
```

Ordem de busca sugerida: `$FT_TEMPLATE_DIR` → `~/.config/ft_cli/templates` (templates do usuário) → `FT_TEMPLATE_DIR` compilado.

Um bônus: `ft new project foo --template=<nome>`, usando outras pastas além de `default/` (ex.: `libft`, `minilibx`).

---

## 3. Arquitetura

### 3.1 `g_commands` definido no header
`app/include/command.h` **define** o array `g_commands[]`. Hoje funciona porque só `dispatcher.c` inclui esse header, mas, se outro `.c` o incluir, o link quebra com "multiple definition".

**Sugestão:**
```c
// command.h
extern const t_command g_commands[];

// commands/registry.c
const t_command g_commands[] = {
    {"new",  cmd_new_execute},
    {"help", cmd_help_execute},
    {NULL, NULL}
};
```

### 3.2 Extrair o `template_engine`
`cmd_new.c` hoje tem cerca de 200 linhas e mistura quatro responsabilidades: resolver o caminho dos templates, substituir placeholders, criar diretórios e escrever arquivos. O README já prevê os módulos `template_engine` e `file_generator`.

**Sugestão:**
```
app/src/template/
    template_render.c    // replace_placeholder, render com lista de variáveis
    template_resolve.c   // localizar pasta de templates (item 2)
app/src/generator/
    generator.c          // cria dirs + renderiza lista de templates
```

API sugerida, genérica o bastante para novas variáveis sem mudar a assinatura:
```c
typedef struct s_tpl_var { const char *key; const char *value; } t_tpl_var;

char *tpl_render(const char *content, const t_tpl_var *vars); // vars termina em {NULL, NULL}
```

Com isso, `cmd_new_execute` vira só orquestração: validar → montar variáveis → chamar o gerador.

### 3.3 Estrutura do projeto gerado como dados, não código
A lista de diretórios e de pares template→saída está *hardcoded* em `cmd_new.c`. Um arquivo `manifest` dentro de cada pasta de template permitiria criar novos templates sem recompilar:

```
# ft_templates/default/manifest
dir  app/includes
dir  app/src
dir  app/obj
file include.tpl   app/includes/{{PROJECT_NAME}}.h
file main.tpl      app/src/main.c
file makefile.tpl  Makefile
file readme.tpl    README.md
file gitignore.tpl .gitignore
```

Um bônus: os caminhos de saída passam pelo mesmo `tpl_render`, o que elimina o caso especial `if (strstr(output_format, ".h"))`.

### 3.4 Parser mais flexível
`parse_args` exige exatamente 4 argumentos, o que impede flags (`--verbose`, `--force`, `--template`) e comandos com outra aridade (`help`, `version`).

**Sugestão:** o parser identifica apenas o comando (`argv[1]`) e separa as flags globais; cada comando valida os próprios argumentos. Assim `t_cli_context` guarda `argc`/`argv` restantes, e `verbose` (hoje nunca usado) passa a ter efeito.

### 3.5 Remover o código legado
Depois que as validações do item 1.2 forem migradas, dá para apagar:
- `t_cli`, `init_and_free_cli.c`, `input_validation.c`, `is_valid_*.c`
- `core/building_project.c`, `core/write_in_files.c`
- As strings `MAKEFILE`, `README`, `DOT_H` e `MAIN` de `files_contents.h`, que foram substituídas pelos `.tpl`. `FALLBACK`/`HELP` podem ir para um `messages.h`.
- O `main` comentado em `main.c`
- As funções de `utils/cli_utils.c` que não forem mais usadas (`join`, `strmapi`)

### 3.6 Headers
- `ft_cli.h` e `fs.h` repetem o mesmo bloco de includes do sistema. Cada header deveria incluir só o que usa.
- Misturar `#include <fs.h>`, `"ft_cli.h"` e `"../../include/ft_cli.h"` é inconsistente. Como o Makefile já passa `-I app/include`, o ideal é usar sempre `#include "fs.h"`.

---

## 4. Consistência e qualidade

- **Mensagens:** hoje há erros em `stdout` (`dispatcher.c`) e em `stderr` (`cmd_new.c`), além de mistura de inglês e português ("Error creating directory" vs "Erro ao ler template"). Escolha um idioma e mande todos os erros para `stderr`.
- **Estilo:** o código antigo segue a Norminette (tabs, declarações no topo, `t_` prefix), enquanto o novo usa 4 espaços e `for (int i = 0; ...)`. Se a ideia for manter o padrão 42, rode `norminette` no CI. Se não, adote um `.clang-format`.
- **Códigos de saída:** padronize (`0` sucesso, `1` erro de uso, `2` erro de I/O etc.).

---

## 5. Makefile (do projeto e do template)

- `.PHONY= all ...` deveria ser `.PHONY: all ...`. Do jeito atual, a linha é só uma atribuição de variável.
- Colocar `$(NAME)` em `.PHONY` força relink a cada `make`.
- O alvo `obj_dir` do Makefile gerado (`makefile.tpl`) imprime "Nothing to be done" de forma enganosa. Um `mkdir -p` na regra de objeto resolve, como já é feito no Makefile principal.
- Adicionar `-MMD -MP` para recompilar quando um header mudar.
- Adicionar alvos `uninstall` (item 2) e `test` (item 6).

---

## 6. Testes e CI

Hoje não há testes, e os workflows em `.github/workflows/` só abrem PRs automaticamente. Além disso, eles usam `github.head_ref`, que fica vazio em eventos de `push`.

**Sugestão:**
1. Testes de integração em shell (`tests/run.sh`): rodar `ft new project foo` num diretório temporário, conferir a árvore gerada, rodar `make` dentro de `foo/` e executar o binário.
2. Casos negativos: nome inválido, projeto já existente, argumentos faltando, `--help`.
3. Testes unitários pequenos para `tpl_render` (placeholders repetidos, placeholder ausente, string vazia).
4. Workflow de CI em PRs: `make` → `make test` → build com `-fsanitize=address,undefined` → `valgrind` (opcional) → `norminette` (opcional).
5. Corrigir `origin-branch` nos workflows existentes para `${{ github.ref_name }}`.

---

## 7. Funcionalidades futuras (a arquitetura acima facilita)

| Comando | Descrição |
|---|---|
| `ft help [comando]` | Ajuda geral e por comando |
| `ft version` | Versão da CLI |
| `ft new project <nome> --template=<t>` | Escolher o template |
| `ft new module <nome>` | Dentro de um projeto, criar `app/src/<nome>/<nome>.c` + header e adicionar ao `SRC` do Makefile |
| `ft init` | Transformar o diretório atual em um projeto ft (sem criar pasta nova) |
| `ft new project <nome> --git` | Rodar `git init` após gerar |
| `ft list templates` | Listar os templates disponíveis |

Com o registry de comandos, cada item vira um novo `cmd_*.c` e uma linha na tabela.

---

## Ordem sugerida de execução

1. Corrigir os bugs da seção 1 (help, validação, sobrescrita, `.gitignore`).
2. Mover `g_commands` para um `.c` (3.1) e remover o código legado (3.5).
3. Extrair o `template_engine` (3.2) e adicionar os testes de integração (6.1–6.2).
4. Corrigir a localização dos templates e o `install`/`uninstall` (2, 5).
5. Adotar o parser flexível (3.4) e o manifest de templates (3.3).
6. Implementar as novas funcionalidades (7).

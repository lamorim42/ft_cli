# SPEC: ft_cli

- **Versão do documento:** 0.1 (rascunho)
- **Data:** 2026-10-01
- **Status:** Congelado em 2026-10-02. A fonte de verdade agora é o OpenSpec (`openspec/specs/`); este documento serve de backlog para os changes. Não edite.
- **Relacionados:** [TDD](TDD.md), [ADRs](adr/README.md)

Este documento define **o que** a `ft` faz do ponto de vista de quem a usa: comandos, argumentos, saídas, códigos de saída, estrutura gerada e formato dos templates. **Como** isso é implementado fica no [TDD](TDD.md).

As palavras **DEVE**, **NÃO DEVE**, **DEVERIA** e **PODE** seguem o sentido da RFC 2119.

---

## 1. Escopo

### 1.1 Objetivo

A `ft` é uma ferramenta de linha de comando que cria a estrutura inicial de projetos em C (diretórios, `Makefile`, `main.c`, header, `README.md` e `.gitignore`) a partir de templates que o usuário pode trocar.

### 1.2 Versões

| Versão | Conteúdo |
|---|---|
| **v1.0** | `help`, `version`, `new project` (com `--force`, `--template` e `--git`), `list templates`, templates externos com `manifest` |
| **Futuro** | `new module` e `init` (seção 5.5 e 5.6, especificação preliminar) |

### 1.3 Plataformas

- Linux e macOS. A `ft` DEVE funcionar nos dois.
- Sem dependências além da libc e de POSIX.1-2008.
- O projeto gerado DEVE compilar com `make` usando `clang` ou `cc`.

---

## 2. Sintaxe geral

```
ft [flags globais] <comando> [argumentos] [flags do comando]
```

- As flags podem vir antes ou depois dos argumentos posicionais.
- Flags com valor aceitam as formas `--flag=valor` e `--flag valor`.
- `--` encerra o processamento de flags. Tudo o que vier depois é argumento posicional.
- Sem nenhum argumento, a `ft` DEVE imprimir a mensagem curta de uso em `stderr` e sair com código `1`.

## 3. Flags globais

| Flag | Efeito |
|---|---|
| `-h`, `--help` | Equivale a `ft help [comando]`. Se vier depois de um comando, mostra a ajuda daquele comando. |
| `-V`, `--version` | Equivale a `ft version`. |
| `-v`, `--verbose` | Mostra detalhes extras na saída (diretório de templates resolvido, cada template lido etc.). |

## 4. Códigos de saída

| Código | Nome | Quando |
|---|---|---|
| `0` | `FT_OK` | Sucesso |
| `1` | `FT_EUSAGE` | Comando desconhecido, argumento faltando ou inválido, flag desconhecida, nome inválido |
| `2` | `FT_EIO` | Erro de sistema de arquivos: destino já existe, sem permissão, template ou manifest não encontrado ou inválido, falha de leitura ou escrita |
| `3` | `FT_EINTERNAL` | Falha de memória ou erro inesperado |

**Regras de saída:**

- Toda mensagem de erro DEVE ir para `stderr`, no formato `ft: <mensagem>`.
- A saída normal (ajuda, listagens, arquivos criados) DEVE ir para `stdout`.
- As mensagens são em português (ADR-0007). Os textos dos exemplos deste documento são os definitivos.

---

## 5. Comandos

### 5.1 `ft help [comando]`

- Sem argumento, DEVE listar os comandos disponíveis com uma linha de descrição cada, em `stdout`, e sair com `0`.
- Com um comando válido, DEVE mostrar a sintaxe e as flags daquele comando.
- Com um comando desconhecido, DEVE escrever `ft: comando desconhecido: <x>` em `stderr` e sair com `1`.

### 5.2 `ft version`

- DEVE imprimir `ft <versão>` (ex.: `ft 1.0.0`) em `stdout` e sair com `0`.

### 5.3 `ft new project <nome> [--template=<t>] [--force] [--git]`

Cria o diretório `./<nome>` com a estrutura descrita pelo template.

| Flag | Padrão | Efeito |
|---|---|---|
| `--template=<t>` | `default` | Usa o template `<t>` (seção 7) |
| `--force` | desligado | Permite gerar dentro de um `<nome>` que já existe, sobrescrevendo os arquivos do template. Arquivos que não fazem parte do template não são tocados. |
| `--git` | desligado | Roda `git init` dentro do projeto depois de gerado. Se o `git` não estiver no `PATH`, o projeto é mantido, um aviso é emitido e a saída é `0`. |

**Validações, feitas nesta ordem e antes de qualquer escrita em disco:**

1. O tipo DEVE ser `project`. Caso contrário: erro de uso, código `1`.
2. O `<nome>` DEVE obedecer à seção 6. Caso contrário: erro de uso, código `1`.
3. O template DEVE existir e o manifest DEVE ser válido. Caso contrário: código `2`.
4. Sem `--force`, `./<nome>` NÃO DEVE existir, seja como diretório, arquivo ou link. Se existir: `ft: '<nome>' já existe (use --force para sobrescrever)`, código `2`.

**Atomicidade:** sem `--force`, se a geração falhar no meio, a `ft` NÃO DEVE deixar um `./<nome>` parcialmente criado.

**Saída de sucesso** (`stdout`):

```
criado  minishell/app/includes/minishell.h
criado  minishell/app/src/main.c
criado  minishell/Makefile
criado  minishell/README.md
criado  minishell/.gitignore
Projeto 'minishell' criado com o template 'default'.
```

Os diretórios não são listados, a não ser com `--verbose`.

### 5.4 `ft list templates`

- DEVE listar, em `stdout`, cada template disponível com o diretório de origem, um por linha:

  ```
  default    /usr/local/share/ft_cli/templates
  libft      /home/luis/.config/ft_cli/templates
  ```

- Um template é qualquer subdiretório que contenha um arquivo `manifest`.
- Se o mesmo nome existir em mais de um diretório, só aparece o que tem precedência (seção 7.1).
- Sai com `0`, mesmo que a lista esteja vazia.

### 5.5 `ft new module <nome>` *(futuro, preliminar)*

- DEVE ser executado na raiz de um projeto gerado pela `ft`, identificada por `Makefile` e `app/src/`. Fora dela, código `1`.
- Cria `app/src/<nome>/<nome>.c` e `app/includes/<nome>.h`.
- Acrescenta `<nome>/<nome>.c` ao `SRC` do `Makefile`, logo após o marcador `# ft:sources`. Se o marcador não existir, os arquivos são criados mesmo assim e um aviso pede que o `Makefile` seja ajustado à mão.

### 5.6 `ft init [--template=<t>] [--force]` *(futuro, preliminar)*

- Gera a estrutura do template no diretório atual, usando o nome do diretório como `<nome>`.
- O nome do diretório DEVE obedecer à seção 6.
- Sem `--force`, aborta se algum arquivo de saída do template já existir.

---

## 6. Regras de nome

Um `<nome>` válido:

- DEVE casar com `^[A-Za-z_][A-Za-z0-9_]*$`;
- DEVE ter no máximo 64 caracteres;
- NÃO DEVE ser uma palavra reservada de C (`int`, `main`, `return`, etc.).

O nome vira diretório, nome do binário, nome do header e include guard. Por isso as regras de identificador C se aplicam.

Quando o nome é inválido, a mensagem DEVERIA sugerir uma correção:

```
ft: nome inválido 'meu-projeto': use apenas letras, números e '_' (ex.: meu_projeto)
```

---

## 7. Templates

### 7.1 Resolução

Para achar o template `<t>`, a `ft` procura `<dir>/<t>/manifest` nos diretórios abaixo, nesta ordem, e usa o primeiro que existir:

1. `$FT_TEMPLATE_DIR`, se a variável estiver definida;
2. `$XDG_CONFIG_HOME/ft_cli/templates`, ou `~/.config/ft_cli/templates` se `XDG_CONFIG_HOME` não estiver definida;
3. o diretório definido na compilação, que por padrão é `$(PREFIX)/share/ft_cli/templates`.

Se nenhum diretório tiver o template, a mensagem de erro DEVE listar os caminhos procurados.

### 7.2 Estrutura de um template

```
<t>/
├── manifest
└── *.tpl
```

### 7.3 Formato do `manifest`

```ebnf
manifest  = { linha } ;
linha     = [ diretiva ] , [ comentario ] , "\n" ;
diretiva  = "dir"  , ws , caminho
          | "file" , ws , caminho_tpl , ws , caminho ;
comentario = "#" , { qualquer caractere exceto "\n" } ;
ws        = ( " " | "\t" ) , { " " | "\t" } ;
```

**Regras:**

- Linhas vazias ou só com espaços são ignoradas.
- `dir <caminho>` cria um diretório relativo à raiz do projeto. Os diretórios intermediários são criados automaticamente.
- `file <tpl> <saída>` lê `<tpl>`, que é relativo à pasta do template, renderiza o conteúdo e grava em `<saída>`, relativo à raiz do projeto. O diretório pai da saída é criado se não existir.
- `<saída>` passa pelo motor de templates, então pode conter placeholders.
- Os caminhos NÃO DEVEM ser absolutos nem conter o componente `..`, inclusive depois da renderização.
- Uma diretiva desconhecida, um número errado de campos ou um caminho inválido gera o erro `ft: <manifest>:<linha>: <motivo>` e código `2`, antes de qualquer escrita.
- Os caminhos não aceitam espaços.

### 7.4 Placeholders

A sintaxe é `{{CHAVE}}`, onde a chave casa com `[A-Z0-9_]+`.

| Chave | Valor | Exemplo para `minishell` |
|---|---|---|
| `PROJECT_NAME` | O nome, como foi digitado | `minishell` |
| `PROJECT_NAME_UPPER` | O nome em maiúsculas | `MINISHELL` |
| `INCLUDE_NAME` | O include guard | `MINISHELL_H` |
| `FT_VERSION` | A versão da `ft` | `1.0.0` |
| `YEAR` | O ano corrente | `2026` |

**Regras:**

- Uma chave desconhecida DEVE ser mantida literalmente na saída e, com `--verbose`, gerar um aviso.
- Uma `{{` sem `}}` correspondente é mantida literalmente.
- A substituição acontece numa única passada: um valor substituído não é processado de novo.

### 7.5 Template `default`

O manifest de referência:

```
# ft_templates/default/manifest
dir  app/includes
dir  app/src
dir  app/obj
dir  app/bin
dir  app/tests
dir  doc/assets

file include.tpl    app/includes/{{PROJECT_NAME}}.h
file main.tpl       app/src/main.c
file makefile.tpl   Makefile
file readme.tpl     README.md
file gitignore.tpl  .gitignore
```

Árvore gerada por `ft new project minishell`:

```
minishell/
├── .gitignore
├── Makefile
├── README.md
├── app/
│   ├── bin/
│   ├── includes/minishell.h
│   ├── obj/
│   ├── src/main.c
│   └── tests/
└── doc/assets/
```

**Requisitos sobre o projeto gerado:**

- `make` na raiz DEVE compilar sem warnings (com `-Wall -Wextra -Werror`) e produzir o binário `./minishell`.
- `./minishell` DEVE imprimir `Hello minishell!` e sair com `0`.
- `make` sem mudanças NÃO DEVE recompilar nem imprimir mensagens enganosas.
- Mudar um header DEVE causar recompilação dos `.c` dependentes.
- O `.gitignore` DEVE ignorar `app/obj/`, `app/bin/` e o binário na raiz.
- O `makefile.tpl` DEVE conter o marcador `# ft:sources` na lista `SRC` (usado na seção 5.5).

---

## 8. Instalação

| Alvo | Efeito |
|---|---|
| `make` | Compila `./ft` |
| `make install` | Instala `ft` em `$(PREFIX)/bin` e os templates em `$(PREFIX)/share/ft_cli/templates` |
| `make uninstall` | Remove os dois |
| `make test` | Roda a suíte de testes |
| `make clean`, `fclean`, `re` | Seguem o padrão de Makefiles da 42 |

- `PREFIX` tem `/usr/local` como padrão e PODE ser trocado: `make install PREFIX=$HOME/.local`.
- O caminho dos templates é fixado na compilação. Por isso `make install PREFIX=x` DEVE recompilar se o `PREFIX` mudou.

---

## 9. Critérios de aceite

| # | Cenário | Resultado esperado |
|---|---|---|
| A1 | `ft` sem argumentos | Uso em `stderr`, código `1` |
| A2 | `ft --help`, `ft -h`, `ft help` | Ajuda em `stdout`, código `0` |
| A3 | `ft help new` | Ajuda do `new`, código `0` |
| A4 | `ft version`, `ft --version` | `ft X.Y.Z`, código `0` |
| A5 | `ft foo` | `ft: comando desconhecido: foo` em `stderr`, código `1` |
| A6 | `ft new project minishell` num diretório vazio | Árvore da seção 7.5, código `0` |
| A7 | Depois de A6: `cd minishell && make && ./minishell` | Compila sem warnings, imprime `Hello minishell!` |
| A8 | Repetir A6 | Erro "já existe", código `2`, nenhum arquivo alterado |
| A9 | A8 com `--force` | Arquivos do template regravados, código `0` |
| A10 | `ft new project meu-projeto` | Erro de nome com sugestão, código `1`, nada criado |
| A11 | `ft new project 1abc`, `../x`, `a/b`, `int` | Erro de nome, código `1`, nada criado |
| A12 | `ft new xyz foo` | Erro de tipo, código `1` |
| A13 | `ft new project` sem nome | Erro de uso, código `1` |
| A14 | `--template=inexistente` | Erro listando os caminhos procurados, código `2`, nada criado |
| A15 | Manifest com diretiva inválida | `ft: <arquivo>:<linha>: ...`, código `2`, nada criado |
| A16 | Manifest com `file x.tpl ../fora` | Erro de caminho, código `2`, nada criado |
| A17 | `FT_TEMPLATE_DIR=/tmp/t ft new project foo` | Usa `/tmp/t/default` |
| A18 | `ft list templates` com um template do usuário de mesmo nome | Mostra só o do usuário |
| A19 | `--git` | `foo/.git` existe, código `0` |
| A20 | Template com um `.tpl` ilegível no meio da geração | Código `2`, `./foo` não existe ao final |
| A21 | Qualquer cenário acima sob ASan/UBSan e valgrind | Sem erros e sem vazamentos |

# ADR-0004: Separar o motor de templates do gerador de arquivos

- **Status:** Proposto
- **Data:** 2026-10-01
- **Relacionados:** ADR-0003, ADR-0005, ADR-0008

## Contexto

O `cmd_new.c` tem cerca de 200 linhas e mistura quatro responsabilidades:

1. localizar a pasta de templates;
2. substituir placeholders (`replace_placeholder`);
3. criar diretórios;
4. ler os templates e escrever os arquivos.

O README já prevê os módulos `template_engine` e `file_generator`. Do jeito atual, `replace_placeholder` só sabe lidar com uma variável fixa, não dá para testar o código isoladamente, e há um caso especial (`if (strstr(output_format, ".h"))`) para o nome do header.

Também há código legado convivendo com o novo: `t_cli`, `init_and_free_cli.c`, `input_validation.c`, `is_valid_*.c`, `core/building_project.c`, `core/write_in_files.c`, as strings `MAKEFILE`/`README`/`DOT_H`/`MAIN` de `files_contents.h` e o `main` comentado.

## Decisão

Separar o código em módulos com responsabilidades únicas:

```
app/src/template/
    template_render.c    // tpl_render: substituição de variáveis
    template_resolve.c   // localização da pasta de templates (ADR-0003)
app/src/generator/
    generator.c          // cria diretórios e renderiza a lista de templates
```

A API de renderização recebe uma lista de variáveis, para que variáveis novas não mudem a assinatura:

```c
typedef struct s_tpl_var {
    const char *key;
    const char *value;
} t_tpl_var;

// vars termina em {NULL, NULL}. Retorna uma string alocada (o chamador libera) ou NULL em caso de erro.
char *tpl_render(const char *content, const t_tpl_var *vars);
```

Os **caminhos de saída** também passam pelo `tpl_render` (ex.: `app/includes/{{PROJECT_NAME}}.h`), o que elimina o caso especial do `.h`.

Com isso, o `cmd_new_execute` só orquestra: **valida, monta as variáveis e chama o gerador**.

Depois da migração (e das validações do ADR-0006), o código legado listado acima é removido, junto com as funções de `utils/cli_utils.c` que ficarem sem uso (`join`, `strmapi`).

## Alternativas consideradas

- **Manter tudo em `cmd_new.c`:** é menos arquivos, mas o comando continua impossível de testar, e cada comando novo que gere arquivos (`new module`, `init`) teria que duplicar a lógica.
- **Usar uma biblioteca de templates (ex.: mustache em C):** traz recursos que o projeto não precisa e uma dependência externa. A substituição simples de `{{CHAVE}}` atende.

## Consequências

- **Positivas:** o `tpl_render` pode ter testes unitários próprios (ADR-0008), `new module` e `init` reaproveitam o gerador, e o código legado sai do projeto.
- **Negativas:** a refatoração mexe em vários arquivos, e convém fazê-la depois de ter testes de integração para pegar regressões. O chamador passa a ser responsável por liberar a string retornada.
- **Regra:** todo retorno `NULL` do `tpl_render` e das funções de I/O deve ser tratado.

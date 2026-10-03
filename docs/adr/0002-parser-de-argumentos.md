# ADR-0002: Parser genérico, com validação de argumentos feita em cada comando

- **Status:** Proposto
- **Data:** 2026-10-01
- **Relacionados:** ADR-0001, ADR-0006

## Contexto

Hoje `parse_args` exige `argc == 4` (`ft <comando> <tipo> <nome>`). Isso:

- impede comandos com outra quantidade de argumentos (`help`, `version`, `init`, `list templates`);
- impede flags (`--force`, `--template=<t>`, `--git`, `--verbose`);
- faz o campo `verbose` de `t_cli_context` nunca ter efeito;
- concentra no parser um conhecimento que pertence a cada comando.

## Decisão

O parser fica genérico e faz só três coisas:

1. Separa as **flags globais** (`--help`/`-h`, `--verbose`, `--version`) e as grava no contexto.
2. Identifica o **comando** (o primeiro argumento posicional).
3. Guarda o `argc`/`argv` restante em `t_cli_context` para o comando consumir.

Cada comando valida os próprios argumentos e flags. O `cmd_new`, por exemplo, confere se o tipo é `project` ou `module`, valida o nome e lê `--force`/`--template`/`--git`.

Esboço do contexto:

```c
typedef struct s_cli_context {
    const char  *command;
    int          argc;     // argumentos depois do comando
    char       **argv;
    bool         verbose;
} t_cli_context;
```

## Alternativas consideradas

- **Continuar com aridade fixa e criar um parser por comando:** isso duplica o tratamento das flags globais.
- **Usar `getopt_long`:** é uma opção válida para as flags. Funciona no Linux e no macOS, mas a permutação de argumentos do GNU tem comportamento diferente entre plataformas, e ele pode ser restrito se o projeto seguir a Norminette. Fica em aberto para a implementação, desde que respeite esta divisão de responsabilidades.

## Consequências

- **Positivas:** o caminho fica aberto para todos os comandos e flags planejados, e o `verbose` passa a funcionar. A mensagem de erro de uso fica perto do comando que conhece a sintaxe.
- **Negativas:** a validação fica espalhada pelos comandos, e cada comando precisa tratar seu próprio erro de uso de forma consistente (ver ADR-0007).

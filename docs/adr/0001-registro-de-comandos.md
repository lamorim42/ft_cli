# ADR-0001: Registro de comandos em tabela, definido num `.c`

- **Status:** Proposto
- **Data:** 2026-10-01
- **Relacionados:** ADR-0002

## Contexto

Na refatoração, o despacho de comandos passou a usar a tabela `g_commands[]`, que associa o nome de cada comando à sua função (`{"new", cmd_new_execute}`). O padrão é bom, mas tem dois problemas:

1. O array é **definido** em `app/include/command.h`. Hoje compila porque só `dispatcher.c` inclui esse header. Se outro `.c` o incluir, o link quebra com "multiple definition".
2. O `help` não está na tabela. Com o `main` novo, `ft --help` cai no `FALLBACK`, e a string `HELP` fica sem uso, embora a própria mensagem de fallback mande o usuário rodar `ft --help`.

Além disso, as funcionalidades planejadas (`help`, `version`, `new module`, `init`, `list templates`) precisam de um jeito barato de adicionar comandos.

## Decisão

- O header só **declara** a tabela, e a definição fica num arquivo próprio:

  ```c
  // app/include/command.h
  extern const t_command g_commands[];

  // app/src/commands/registry.c
  const t_command g_commands[] = {
      {"new",  cmd_new_execute},
      {"help", cmd_help_execute},
      {NULL, NULL}
  };
  ```

- Cada comando fica num `cmd_<nome>.c` com assinatura uniforme e recebe o `t_cli_context`.
- `help` vira um comando como os outros. O parser converte `--help` e `-h` em `ctx->command = "help"` (ver ADR-0002).
- Adicionar um comando novo exige só criar o `cmd_*.c` e acrescentar uma linha em `registry.c`.

## Alternativas consideradas

- **`if/else` com `strcmp` no `main`:** o código antigo usava algo parecido. Fica mais difícil de ler a cada comando novo e mistura o despacho com a lógica.
- **Manter a definição no header com `static`:** cada unidade de compilação ganharia a própria cópia da tabela. Funciona, mas esconde o problema em vez de resolvê-lo.

## Consequências

- **Positivas:** o link não quebra se mais arquivos incluírem `command.h`, o `--help` volta a funcionar, e cada comando novo custa uma linha na tabela.
- **Negativas:** passa a existir mais um arquivo (`registry.c`), e o compilador não acusa um comando esquecido fora da tabela.
- **Trabalho derivado:** criar `cmd_help.c` e mover `FALLBACK`/`HELP` de `files_contents.h` para um `messages.h`.

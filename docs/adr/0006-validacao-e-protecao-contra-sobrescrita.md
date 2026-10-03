# ADR-0006: Validação de entrada e proteção contra sobrescrita

- **Status:** Proposto
- **Data:** 2026-10-01
- **Relacionados:** ADR-0002, ADR-0004

## Contexto

A refatoração perdeu duas proteções:

1. **Validação de entrada:** `is_valid_option`, `is_valid_arg` e `is_valid_name` continuam compilando, mas o fluxo novo não os chama. Com isso:
   - o `ctx->type` é ignorado, e `ft new qualquercoisa foo` cria um projeto;
   - o nome não é validado. `meu-projeto` ou `1abc` geram um include guard inválido (`MEU-PROJETO_H`), e nomes com `/` ou `..` criam diretórios fora do lugar esperado.
2. **Sobrescrita:** quando o diretório já existe, `fs_create_dir` só imprime um erro e o fluxo segue. Depois, `process_template` abre os arquivos com `fopen(..., "w")` e sobrescreve `main.c`, `Makefile` etc. Rodar `ft new project minishell` duas vezes apaga o trabalho do usuário.

## Decisão

1. **Validar nomes numa função reutilizável**, em `utils/`:

   ```c
   // Aceita apenas [A-Za-z_][A-Za-z0-9_]*
   bool validate_identifier(const char *name);
   ```

   O nome vira diretório, nome de binário e include guard, então precisa ser um identificador C válido. Isso também elimina `/` e `..`.

2. **Cada comando valida o próprio tipo** (ADR-0002). O `cmd_new` aceita só os tipos que suporta (`project` e, no futuro, `module`).

3. **Não sobrescrever por padrão:**
   - criar `fs_exists()` (já previsto, comentado em `fs.h`);
   - se a raiz do projeto já existir, abortar antes de criar qualquer coisa, com uma mensagem de erro clara;
   - checar o retorno de `fs_create_dir` e de toda escrita, e interromper no primeiro erro;
   - uma flag `--force` permite sobrescrever de propósito.

## Alternativas consideradas

- **Converter o nome automaticamente** (ex.: `meu-projeto` → `meu_projeto`): é amigável, mas surpreende o usuário e deixa nome de pasta e nome de binário diferentes. Rejeitar com uma mensagem explicativa é mais previsível.
- **Pular só os arquivos que já existem:** gera um projeto meio antigo, meio novo, difícil de entender. Abortar por inteiro é mais seguro.

## Consequências

- **Positivas:** o usuário não perde trabalho por acidente, os projetos gerados sempre compilam, e não há como escrever fora do diretório esperado.
- **Negativas:** nomes com hífen, comuns em repositórios, são rejeitados. A mensagem de erro deve sugerir a forma com `_`.
- **Trabalho derivado:** depois da migração, apagar `input_validation.c` e `is_valid_*.c` (ADR-0004), e cobrir esses casos com testes negativos (ADR-0008).

# ADR-0008: Estratégia de testes e integração contínua

- **Status:** Proposto
- **Data:** 2026-10-01
- **Relacionados:** ADR-0004, ADR-0006, ADR-0007

## Contexto

Na época da análise não havia testes automatizados, e as regressões da refatoração (`--help` quebrado, validação perdida, sobrescrita de projetos, `.gitignore` ausente) passaram sem ser notadas.

Os workflows em `.github/workflows/` só abrem PRs automaticamente e usam `github.head_ref`, que fica vazio em eventos de `push`.

## Decisão

### Testes

1. **Integração, em shell** (`tests/run.sh`), chamado por `make test`:
   - roda `ft new project foo` num diretório temporário;
   - confere a árvore gerada, incluindo o `.gitignore`;
   - roda `make` dentro de `foo/` e executa o binário gerado.
2. **Casos negativos**, cada um com o código de saída esperado (ADR-0007):
   - nome inválido (`meu-projeto`, `1abc`, `../x`);
   - projeto que já existe, com e sem `--force`;
   - argumentos faltando e comando desconhecido;
   - `--help` e `-h`.
3. **Unitários pequenos** para `tpl_render` (ADR-0004): placeholder repetido, placeholder ausente, string vazia e `vars` vazio.

### CI (rodando em cada PR)

1. `make`
2. `make test`
3. Build com `-fsanitize=address,undefined` e os testes de novo
4. *(Opcional)* `valgrind`
5. *(Opcional, conforme ADR-0007)* `norminette` ou `clang-format --dry-run`

### Workflows existentes

Trocar `github.head_ref` por `${{ github.ref_name }}` no `origin-branch`.

## Alternativas consideradas

- **Usar só testes unitários em C** (com Criterion, Unity etc.): não pegariam os bugs reais encontrados, que estão na interação entre parser, comando e sistema de arquivos. Os testes de integração em shell cobrem isso com pouco esforço.
- **Fazer só testes manuais:** foi o que deixou passar as regressões.

## Consequências

- **Positivas:** as regressões da seção de bugs ficam cobertas, e a refatoração do ADR-0004 pode ser feita com segurança. Os sanitizers pegam vazamentos e acessos inválidos.
- **Negativas:** o CI fica mais lento e alguém precisa manter os testes. Testes em shell são menos precisos para isolar onde está a falha.

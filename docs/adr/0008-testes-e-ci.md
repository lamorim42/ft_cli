# ADR-0008: Estratégia de testes e integração contínua

- **Status:** Proposto
- **Data:** 2026-10-01 (biblioteca de testes unitários e testes funcionais definidos em 2026-10-02)
- **Relacionados:** ADR-0004, ADR-0006, ADR-0007

## Contexto

Na época da análise não havia testes automatizados, e as regressões da refatoração (`--help` quebrado, validação perdida, sobrescrita de projetos, `.gitignore` ausente) passaram sem ser notadas.

Os workflows em `.github/workflows/` só abrem PRs automaticamente e usam `github.head_ref`, que fica vazio em eventos de `push`.

## Decisão

### Testes

1. **Integração, em shell** (`tests/run.sh`), chamado por `make test`. No repositório esta camada se chama **funcional**: `tests/functional/test_<cenário>.sh`, com helpers em `tests/functional/lib.sh`, rodados por `make test-functional`. Cada arquivo roda num `mktemp -d` próprio e sai com `0` (passou), `77` (pulado, com o motivo) ou outro código (falhou). Os primeiros testes são snapshots do comportamento atual (A6 ainda sem `.gitignore`, A7 e os defeitos D1–D3), que cada passo da migração atualiza até chegar ao alvo:
   - roda `ft new project foo` num diretório temporário;
   - confere a árvore gerada, incluindo o `.gitignore`;
   - roda `make` dentro de `foo/` e executa o binário gerado.
2. **Casos negativos**, cada um com o código de saída esperado (ADR-0007):
   - nome inválido (`meu-projeto`, `1abc`, `../x`);
   - projeto que já existe, com e sem `--force`;
   - argumentos faltando e comando desconhecido;
   - `--help` e `-h`.
3. **Unitários** com a biblioteca [Unity](https://github.com/ThrowTheSwitch/Unity), copiada em `tests/vendor/unity/`:
   - um arquivo `tests/unit/test_<módulo>.c` por módulo, cada um compilado como executável próprio e rodado por `make test`;
   - os primeiros cobrem `parse_args`, `dispatch_command` e `fs_*`, registrando o comportamento atual;
   - cada change da migração acrescenta os seus, como os de `tpl_render` (ADR-0004): placeholder repetido, placeholder ausente, string vazia e `vars` vazio.

   **Por que Unity:** os critérios foram ser acessível (sem instalação nem `sudo`, em Linux e macOS), fácil de usar e flexível para o Makefile do projeto. A Unity são três arquivos em C puro, sem dependências, com asserts que mostram esperado × obtido. O custo é registrar cada teste no `main` com `RUN_TEST`; declarar os testes como `static` faz o `-Werror` acusar um registro esquecido.

### CI (rodando em cada PR)

1. `make`
2. `make test`
3. Build com `-fsanitize=address,undefined` e os testes de novo
4. *(Opcional)* `valgrind`
5. *(Opcional, conforme ADR-0007)* `norminette` ou `clang-format --dry-run`

### Workflows existentes

Trocar `github.head_ref` por `${{ github.ref_name }}` no `origin-branch`.

## Alternativas consideradas

- **Usar só testes unitários em C:** não pegariam os bugs reais encontrados, que estão na interação entre parser, comando e sistema de arquivos. Por isso unitários e integração convivem: os unitários isolam a lógica de cada módulo, e os testes em shell cobrem o fluxo completo.
- **Outras bibliotecas de testes unitários:**
  - *Criterion:* a mais ergonômica (testes se registram sozinhos, cada um num processo), mas exige a lib instalada em cada máquina e no CI, o que é uma barreira sem `sudo`.
  - *cmocka:* também exige instalação, e o ponto forte dela, os mocks, não é necessário aqui.
  - *acutest:* um header só e portátil, mas os asserts são genéricos (`TEST_CHECK(strcmp(...) == 0)`) e as mensagens de falha não mostram os valores.
  - *`assert` puro* (proposta original do TDD §6): para no primeiro erro, não mostra esperado × obtido e não tem `setUp`/`tearDown`.
- **Fazer só testes manuais:** foi o que deixou passar as regressões.

## Consequências

- **Positivas:** as regressões da seção de bugs ficam cobertas, e a refatoração do ADR-0004 pode ser feita com segurança. Os sanitizers pegam vazamentos e acessos inválidos.
- **Negativas:** o CI fica mais lento e alguém precisa manter os testes. Testes em shell são menos precisos para isolar onde está a falha.

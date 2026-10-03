# Proposal

## Why

Os testes unitários do change `add-unit-test-framework` cobrem `parse_args`, `dispatch_command` e `fs_*` isoladamente, mas nada verifica a `ft` de ponta a ponta: rodar o binário, conferir a árvore gerada e compilar o projeto criado. As regressões da branch `feature/refatorando` (ADR-0008) estavam justamente na interação entre parser, comando e sistema de arquivos. Antes de começar os passos 2 a 12 da migração do `docs/TDD.md`, precisamos de um teste de snapshot que diga se `ft new project` continua funcionando depois de cada passo. Este change é o passo 1 do TDD §7.

## What Changes

- **Testes funcionais em shell POSIX** (o que o TDD §6 e o ADR-0008 chamam de "integração"): `tests/run.sh` roda cada `tests/functional/test_*.sh` num diretório temporário próprio, chamando o binário `./ft` do repositório, e falha se algum falhar.
- Os testes funcionais registram o comportamento **atual** da `ft`, inclusive o que a migração vai mudar, para que cada change troque a expectativa de forma explícita:
  - A6 (snapshot atual): `ft new project minishell` num diretório vazio sai com `0`, cria a árvore atual (sem `.gitignore`, defeito D4) e imprime as mensagens atuais;
  - A7: dentro do projeto gerado, `make` compila sem warnings e `./minishell` imprime `Hello minishell!`;
  - casos atuais de erro e de defeito conhecido: `ft` sem argumentos e `ft --help` (D1) saem com `1` e o uso em `stderr`; `ft foo a b` sai com `1`; repetir A6 sobrescreve sem aviso (D3); `ft new project meu-projeto` é aceito (D2); `ft new xyz foo` ignora o tipo.
- **Testes unitários complementares** para as funções públicas que já existem, sem tocar em código de produção: casos de borda de `fs_create_dir`, `fs_create_file`, `fs_read_file`, `parse_args` e `dispatch_command` que ainda não estão cobertos (ver `design.md`).
- **Makefile:** novo alvo `make test-functional`; `make test` passa a rodar os unitários e os funcionais e só falha no fim, para que uma falha numa suíte não esconda o resultado da outra.
- **Documentação:** seção "Testes" do `README.md` e ADR-0008 atualizados com a suíte funcional.

Fora do escopo:

- Testes do comportamento-alvo do SPEC (A1–A5, A8–A21). Cada passo da migração acrescenta os seus e atualiza os snapshots deste change.
- Mudança de comportamento da `ft`: nenhuma, nem correção dos defeitos D1–D8.
- Extrair as funções `static` de `cmd_new.c` para testá-las (o passo 6 as substitui pelo `tpl_render`).
- CI, alvo `sanitize` e `valgrind` (passo 12 do TDD).

## Capabilities

### New Capabilities

Nenhuma. O change só adiciona testes e não altera comportamento observável da `ft`, por isso declara `skip_specs: true`.

### Modified Capabilities

Nenhuma.

## Impact

- **Código de produção:** nenhuma alteração em `app/src/`, `app/include/` ou `ft_templates/`.
- **Testes:** novos `tests/run.sh`, `tests/functional/lib.sh` e `tests/functional/test_*.sh`; novos casos em `tests/unit/test_fs.c`, `test_parser.c` e `test_dispatcher.c`.
- **Build:** `Makefile` ganha `test-functional` e muda a receita de `test`.
- **Plataformas:** os testes de geração dependem de `/proc/self/exe` (defeito D6) e são pulados fora do Linux até o passo 9. O A7 depende de `clang`, que o `makefile.tpl` atual exige, e é pulado se ele não estiver no `PATH`.
- **Documentação:** `README.md` e `docs/adr/0008-testes-e-ci.md`.

# Architecture Decision Records: ft_cli

Este diretório registra as decisões de arquitetura do `ft_cli`. Cada arquivo trata de uma decisão e segue o formato de Michael Nygard: contexto, decisão, alternativas e consequências.

Este diretório complementa o [SPEC](../SPEC.md), que define o comportamento da CLI, e o [TDD](../TDD.md), que define o design de implementação.

As decisões abaixo saíram da análise da branch `feature/refatorando` (commit `85bce6c`) registrada em [`plan-claude.md`](../archive/plan-claude.md).

Desde 2026-10-02 a fonte de verdade do comportamento é o OpenSpec (`openspec/specs/`), e o SPEC e o TDD estão congelados. Os ADRs continuam vivos: o `design.md` de cada change cita os ADRs relacionados e, quando o change é arquivado, os ADRs totalmente implementados passam a **Aceito**. Todas começam como **Proposto**. Quando uma for implementada, mude o status para **Aceito**. Se alguma for descartada ou trocada por outra, marque-a como **Rejeitado** ou **Substituído por ADR-XXXX**.

| ADR | Título | Status |
|---|---|---|
| [0001](0001-registro-de-comandos.md) | Registro de comandos em tabela, definido num `.c` | Proposto |
| [0002](0002-parser-de-argumentos.md) | Parser genérico, com validação de argumentos feita em cada comando | Proposto |
| [0003](0003-localizacao-dos-templates.md) | Localização e instalação dos templates | Proposto |
| [0004](0004-motor-de-templates.md) | Separar o motor de templates do gerador de arquivos | Proposto |
| [0005](0005-manifest-de-templates.md) | Descrever a estrutura do projeto gerado num `manifest` | Proposto |
| [0006](0006-validacao-e-protecao-contra-sobrescrita.md) | Validação de entrada e proteção contra sobrescrita | Proposto |
| [0007](0007-convencoes-de-erro-e-estilo.md) | Convenções de mensagens, códigos de saída e estilo | Proposto |
| [0008](0008-testes-e-ci.md) | Estratégia de testes e integração contínua | Proposto |

## Ordem de implementação sugerida

1. ADR-0006 e o comando `help` do ADR-0001 (corrigem as regressões).
2. ADR-0001 completo e a remoção do código legado.
3. ADR-0004 e os testes de integração do ADR-0008.
4. ADR-0003.
5. ADR-0002 e ADR-0005.
6. As novas funcionalidades (`version`, `new module`, `init`, `list templates`, etc.).

## Fora do escopo dos ADRs

Algumas correções pontuais do plano não são decisões de arquitetura e devem ser feitas direto como bugfix:

- Criar `ft_templates/default/gitignore.tpl`, que o README promete mas não existe.
- Checar os retornos `NULL` de `replace_placeholder`, e de `ftell`/`fread` em `fs_read_file`.
- No Makefile: trocar `.PHONY=` por `.PHONY:`, tirar `$(NAME)` do `.PHONY`, adicionar `-MMD -MP` e corrigir o alvo `obj_dir` do `makefile.tpl`.
- Padronizar os includes como `#include "fs.h"` e fazer cada header incluir só o que usa.

## Novo ADR

Copie o modelo abaixo para `NNNN-titulo-curto.md`, usando o próximo número livre:

```markdown
# ADR-NNNN: Título

- **Status:** Proposto
- **Data:** AAAA-MM-DD

## Contexto
## Decisão
## Alternativas consideradas
## Consequências
```

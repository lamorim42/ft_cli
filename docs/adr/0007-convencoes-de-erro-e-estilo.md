# ADR-0007: Convenções de mensagens, códigos de saída e estilo

- **Status:** Proposto
- **Data:** 2026-10-01 (pendências resolvidas em 2026-10-02)

## Contexto

O código atual mistura convenções:

- **Saída de erro:** `dispatcher.c` escreve erros em `stdout`, enquanto `cmd_new.c` usa `stderr`.
- **Idioma:** há mensagens em inglês e em português ("Error creating directory" vs. "Erro ao ler template").
- **Códigos de saída:** não há padrão.
- **Estilo:** o código antigo segue a Norminette (tabs, declarações no topo da função, prefixo `t_` em tipos), e o novo usa 4 espaços e `for (int i = 0; ...)`.
- **Includes:** há `#include <fs.h>`, `"ft_cli.h"` e `"../../include/ft_cli.h"`, embora o Makefile já passe `-I app/include`.

## Decisão

1. **Toda mensagem de erro vai para `stderr`.** A saída normal (ajuda, listagens, mensagem de sucesso) vai para `stdout`.
2. **Mensagens em português**, que é o idioma do público atual. Identificadores no código continuam em inglês.
3. **Códigos de saída padronizados**, definidos como constantes num header:

   | Código | Significado |
   |---|---|
   | `0` | Sucesso |
   | `1` | Erro de uso (argumento inválido, comando desconhecido) |
   | `2` | Erro de I/O (arquivo, diretório, template não encontrado) |
   | `3` | Erro interno (falta de memória, erro inesperado) |

4. **Estilo definido por um `.clang-format`** na raiz do repositório e checado no CI. O projeto não segue a Norminette. Só são reformatados os arquivos que sobrevivem à remoção do legado.
5. **Includes:** sempre na forma `#include "fs.h"`, e cada header inclui só o que usa.

## Alternativas consideradas

- **Não fixar convenções:** cada arquivo continuaria com seu próprio padrão, e as mensagens com idioma e destino aleatórios.
- **Mensagens em inglês:** é o padrão de ferramentas de linha de comando, mas o público da `ft` é lusófono.
- **Norminette:** proíbe `for`, `switch` e `goto` e limita as funções a 25 linhas. A `ft` é uma ferramenta para alunos da 42, não um projeto avaliado por ela, então essas restrições não se justificam.

## Consequências

- **Positivas:** scripts podem confiar no código de saída e redirecionar `stderr`, a interface fica coerente e as revisões discutem lógica, não formatação.
- **Negativas:** todas as mensagens existentes precisam ser revisadas de uma vez. Quem tiver o hábito da Norminette precisa se acostumar a outro estilo.

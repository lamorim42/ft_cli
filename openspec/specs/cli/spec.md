# cli Specification

## Purpose

Define a interface de linha de comando da `ft`: os códigos de saída, para onde vão as mensagens normais e de erro, e como a `ft` reage a um comando que não conhece.

## Requirements

### Requirement: Códigos de saída padronizados
A `ft` MUST sair com `0` em caso de sucesso, `1` em erro de uso (argumentos faltando ou comando desconhecido), `2` em erro de sistema de arquivos (template não encontrado ou ilegível, arquivo ou diretório que não pode ser criado) e `3` em erro interno (falta de memória).

#### Scenario: Sucesso
- **WHEN** o usuário roda `ft new project minishell` num diretório vazio
- **THEN** a `ft` sai com código `0`

#### Scenario: Erro de uso
- **WHEN** o usuário roda `ft foo a b`
- **THEN** a `ft` sai com código `1`

#### Scenario: Erro de sistema de arquivos
- **WHEN** o usuário roda `ft new project a/b` num diretório sem `a/`
- **THEN** a `ft` sai com código `2`

### Requirement: Mensagens de erro em stderr com prefixo
Toda mensagem de erro MUST ir para `stderr`, em português, no formato `ft: <mensagem>`. Avisos MUST usar o formato `ft: aviso: <mensagem>`. A saída normal MUST ir para `stdout`, sem prefixo. A mensagem de uso exibida quando faltam argumentos é a exceção: continua sem o prefixo até o comando `help` existir.

#### Scenario: Erro não aparece em stdout
- **WHEN** o usuário roda `ft foo a b`
- **THEN** `stdout` fica vazio
- **AND** a mensagem de erro aparece em `stderr` começando com `ft: `

#### Scenario: Sem argumentos (A1)
- **WHEN** o usuário roda `ft` sem argumentos
- **THEN** a mensagem de uso aparece em `stderr`
- **AND** `stdout` fica vazio
- **AND** a `ft` sai com código `1`

### Requirement: Comando desconhecido
Quando o comando não existe, a `ft` MUST escrever `ft: comando desconhecido: <comando>` em `stderr` e sair com código `1`, sem criar nada em disco.

#### Scenario: Comando desconhecido (A5)
- **WHEN** o usuário roda `ft foo a b`
- **THEN** `stderr` contém exatamente a linha `ft: comando desconhecido: foo`
- **AND** `stdout` fica vazio
- **AND** a `ft` sai com código `1`
- **AND** nenhum arquivo ou diretório é criado

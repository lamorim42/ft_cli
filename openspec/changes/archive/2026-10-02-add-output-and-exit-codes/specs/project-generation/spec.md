# Spec Delta

## Purpose

Define o comando `ft new project`: o que ele imprime ao gerar a estrutura de um projeto em C e como reporta as falhas que acontecem durante a geração.

## ADDED Requirements

### Requirement: Saída de sucesso
Ao gerar um projeto, a `ft` MUST imprimir em `stdout` uma linha `criado  <caminho>` (dois espaços depois de `criado`) para cada arquivo gerado, na ordem de criação, e no fim a linha `Projeto '<nome>' criado com o template 'default'.`. Diretórios não são listados. `stderr` MUST ficar vazio.

#### Scenario: Projeto novo (A6, sem .gitignore)
- **WHEN** o usuário roda `ft new project minishell` num diretório vazio
- **THEN** `stdout` é exatamente:
  ```
  criado  minishell/app/includes/minishell.h
  criado  minishell/app/src/main.c
  criado  minishell/Makefile
  criado  minishell/README.md
  Projeto 'minishell' criado com o template 'default'.
  ```
- **AND** `stderr` fica vazio
- **AND** a `ft` sai com código `0`

### Requirement: Diretório existente não é erro
Um diretório do projeto que já existe MUST ser aceito sem mensagem: a geração continua e os arquivos do template são regravados.

#### Scenario: Gerar de novo sobre o mesmo projeto
- **WHEN** o usuário roda `ft new project minishell` duas vezes no mesmo diretório
- **THEN** a segunda execução sai com código `0`
- **AND** `stderr` fica vazio

### Requirement: Falha ao criar diretório ou arquivo
Se um diretório ou arquivo do projeto não puder ser criado por um motivo diferente de já existir, a `ft` MUST parar a geração, escrever `ft: não foi possível criar o diretório <caminho>: <motivo>` ou `ft: não foi possível criar o arquivo <caminho>: <motivo>` em `stderr`, onde `<motivo>` é a descrição do erro do sistema, e sair com código `2`.

#### Scenario: Diretório pai inexistente
- **WHEN** o usuário roda `ft new project a/b` num diretório sem `a/`
- **THEN** `stderr` contém exatamente a linha `ft: não foi possível criar o diretório a/b: No such file or directory`
- **AND** nenhuma linha `criado` aparece em `stdout`
- **AND** a `ft` sai com código `2`

### Requirement: Falha ao ler template
Se um template não puder ser lido, a `ft` MUST parar a geração, escrever `ft: não foi possível ler o template <caminho>: <motivo>` em `stderr` e sair com código `2`. Arquivos gerados antes da falha podem permanecer (a geração atômica não faz parte deste comportamento).

#### Scenario: Template sem permissão de leitura
- **WHEN** o template `main.tpl` não tem permissão de leitura e o usuário roda `ft new project foo`
- **THEN** `stderr` contém uma linha que começa com `ft: não foi possível ler o template ` e termina com `main.tpl: Permission denied`
- **AND** `stdout` não contém `criado  foo/app/src/main.c`
- **AND** a `ft` sai com código `2`

### Requirement: Templates não localizados
Se a `ft` não conseguir determinar o diretório de onde lê os templates, MUST escrever `ft: não foi possível localizar os templates: <motivo>` em `stderr`, sair com código `2` e não criar nada em disco.

#### Scenario: Diretório do executável indisponível
- **WHEN** a `ft` não consegue resolver o caminho do próprio executável
- **THEN** `stderr` contém uma linha que começa com `ft: não foi possível localizar os templates: `
- **AND** a `ft` sai com código `2`
- **AND** nenhum arquivo ou diretório é criado

### Requirement: Falta de memória
Se uma alocação falhar durante a geração, a `ft` MUST escrever `ft: memória insuficiente` em `stderr` e sair com código `3`, sem usar o resultado da alocação que falhou.

#### Scenario: Alocação falha ao montar o conteúdo
- **WHEN** uma alocação falha ao preparar o nome em maiúsculas ou ao substituir os placeholders de um template
- **THEN** `stderr` contém exatamente a linha `ft: memória insuficiente`
- **AND** a `ft` sai com código `3`

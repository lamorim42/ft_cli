# ADR-0005: Descrever a estrutura do projeto gerado num `manifest`

- **Status:** Proposto
- **Data:** 2026-10-01
- **Relacionados:** ADR-0003, ADR-0004

## Contexto

As listas de diretórios a criar e de pares template → arquivo de saída estão *hardcoded* em `cmd_new.c`. Por isso:

- criar um template novo (`libft`, `minilibx`) exige mudar o código e recompilar;
- esquecer de registrar um arquivo passa despercebido, como aconteceu com o `.gitignore`, que o README promete mas não é gerado;
- os templates do usuário (`~/.config/ft_cli/templates`, ADR-0003) não podem ter outra estrutura.

## Decisão

Cada pasta de template ganha um arquivo `manifest`, em texto, que o gerador lê:

```
# ft_templates/default/manifest
dir  app/includes
dir  app/src
dir  app/obj
file include.tpl   app/includes/{{PROJECT_NAME}}.h
file main.tpl      app/src/main.c
file makefile.tpl  Makefile
file readme.tpl    README.md
file gitignore.tpl .gitignore
```

Regras do formato:

- uma diretiva por linha, com campos separados por espaços em branco;
- linhas vazias e linhas começando com `#` são ignoradas;
- `dir <caminho>` cria um diretório relativo à raiz do projeto;
- `file <template> <saída>` renderiza o template e grava o resultado na saída. Os dois caminhos são relativos, e a saída passa pelo `tpl_render`;
- caminhos absolutos ou com `..` são rejeitados;
- uma diretiva desconhecida aborta a geração com erro informando a linha.

## Alternativas consideradas

- **Manter as listas no código:** é mais simples hoje, mas cada template novo exige recompilar.
- **Gerar a árvore copiando a pasta do template:** não precisa de manifest, mas mistura arquivos de controle com os de saída e não permite renomear arquivos com variáveis.
- **Usar JSON/YAML:** é mais expressivo, mas exigiria um parser ou uma dependência externa em C. O formato de linhas basta.

## Consequências

- **Positivas:** templates novos não exigem recompilar, cada template pode ter sua própria estrutura, o `.gitignore` volta a ser gerado, e o comando `ft list templates` fica fácil de implementar.
- **Negativas:** é mais um parser para manter e testar, e um manifest mal escrito vira erro em tempo de execução. A validação de caminhos é obrigatória por segurança.

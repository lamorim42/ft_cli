# ADR-0003: Localização e instalação dos templates

- **Status:** Proposto
- **Data:** 2026-10-01
- **Relacionados:** ADR-0004, ADR-0005

## Contexto

Hoje o `make install` copia `ft_templates/` para **dentro de `/usr/local/bin`**, e o `cmd_new` descobre a pasta dos templates lendo `/proc/self/exe`. Problemas:

- `/usr/local/bin` deve conter apenas executáveis.
- `/proc/self/exe` só existe no Linux, então não funciona no macOS, que muitos alunos da 42 usam.
- O buffer `exe_path[512]` pode truncar caminhos longos.
- Não existe `make uninstall`.
- O usuário não tem como usar templates próprios sem mexer na instalação.

## Decisão

1. **Caminho definido na compilação**, seguindo a convenção FHS:

   ```make
   PREFIX       ?= /usr/local
   TEMPLATE_DIR  = $(PREFIX)/share/ft_cli/templates
   CFLAGS       += -DFT_TEMPLATE_DIR=\"$(TEMPLATE_DIR)\"

   install: $(NAME)
   	install -d $(PREFIX)/bin $(TEMPLATE_DIR)
   	install -m 755 $(NAME) $(PREFIX)/bin/
   	cp -r ft_templates/* $(TEMPLATE_DIR)/

   uninstall:
   	rm -f $(PREFIX)/bin/$(NAME)
   	rm -rf $(PREFIX)/share/ft_cli
   ```

2. **Ordem de busca.** Para cada template, a `ft` procura `<base>/<nome>/manifest` nas bases abaixo e usa a primeira que tiver o template. A precedência é por template, então um template do usuário substitui só o de mesmo nome, e os demais instalados continuam visíveis.
   1. a variável de ambiente `$FT_TEMPLATE_DIR`;
   2. `$XDG_CONFIG_HOME/ft_cli/templates`, ou `~/.config/ft_cli/templates` se a variável não estiver definida, para templates do usuário;
   3. a constante `FT_TEMPLATE_DIR`, definida na compilação.

3. Cada subpasta é um template nomeado (`default/`, `libft/`, `minilibx/`...), escolhido com `--template=<nome>` (o padrão é `default`).

4. A resolução fica num módulo próprio (`template_resolve.c`, ver ADR-0004).

## Alternativas consideradas

- **Continuar com `/proc/self/exe`:** só funciona no Linux e mantém os templates misturados com os binários.
- **Embutir os templates no binário** (como strings, a exemplo do antigo `files_contents.h`): elimina a dependência de arquivos, mas exige recompilar a cada mudança de template e impede templates do usuário. Foi justamente o que a refatoração abandonou.

## Consequências

- **Positivas:** funciona no Linux e no macOS, segue as convenções de instalação, tem `uninstall`, aceita `PREFIX=$HOME/.local` (útil em máquinas da 42 sem `sudo`) e permite templates personalizados.
- **Negativas:** se o binário for movido sem os templates, ele deixa de achá-los, e por isso a mensagem de erro precisa listar os caminhos procurados. A ordem de busca também precisa ser documentada no README.

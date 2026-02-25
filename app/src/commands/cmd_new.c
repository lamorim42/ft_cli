#include "../../include/ft_cli.h"
#include "../../include/fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>

// Função para converter string para maiúscula
static char *str_to_upper(const char *str)
{
	char *result = malloc(strlen(str) + 1);
	if (!result)
		return NULL;
	for (int i = 0; str[i]; i++)
		result[i] = toupper((unsigned char)str[i]);
	result[strlen(str)] = '\0';
	return result;
}

// Função para substituir placeholders em um string
static char *replace_placeholder(const char *content, const char *placeholder, const char *replacement)
{
	size_t content_len = strlen(content);
	size_t placeholder_len = strlen(placeholder);
	size_t replacement_len = strlen(replacement);
	
	// Contar ocorrências
	int count = 0;
	const char *temp = content;
	while ((temp = strstr(temp, placeholder)) != NULL) {
		count++;
		temp += placeholder_len;
	}
	
	if (count == 0)
		return strdup(content);
	
	// Alocar espaço para o novo conteúdo
	size_t new_size = content_len + count * (replacement_len - placeholder_len) + 1;
	char *result = malloc(new_size);
	if (!result)
		return NULL;
	
	// Fazer as substituições
	const char *src = content;
	char *dst = result;
	const char *found;
	
	while ((found = strstr(src, placeholder)) != NULL) {
		// Copiar tudo até o placeholder
		size_t len = found - src;
		strncpy(dst, src, len);
		dst += len;
		
		// Copiar o replacement
		strcpy(dst, replacement);
		dst += replacement_len;
		
		// Avançar src
		src = found + placeholder_len;
	}
	
	// Copiar o resto
	strcpy(dst, src);
	return result;
}

// Função para criar um diretório dentro do projeto
static void create_project_dir(const char *project_name, const char *subdir)
{
	char path[512];
	snprintf(path, sizeof(path), "%s/%s", project_name, subdir);
	fs_create_dir(path);
}

// Função para processar e escrever um template
static int process_template(const char *template_path, const char *output_path, 
							const char *project_name, const char *include_guard)
{
	char *content = fs_read_file(template_path);
	if (!content) {
		fprintf(stderr, "Erro ao ler template: %s\n", template_path);
		return 1;
	}
	
	// Fazer substituições
	char *temp = replace_placeholder(content, "{{PROJECT_NAME}}", project_name);
	free(content);
	
	char *final = replace_placeholder(temp, "{{INCLUDE_NAME}}", include_guard);
	free(temp);
	
	// Escrever arquivo
	FILE *out = fopen(output_path, "w");
	if (!out) {
		fprintf(stderr, "Erro ao criar arquivo: %s\n", output_path);
		free(final);
		return 1;
	}
	
	fputs(final, out);
	fclose(out);
	free(final);
	
	printf("Criado arquivo: %s\n", output_path);
	return 0;
}

// Função para processar múltiplos templates
static int process_templates(const char *exe_dir, const char *project_name, 
							 const char *include_guard)
{
	struct {
		const char *tpl_file;
		const char *output_format;
	} templates[] = {
		{"include.tpl", "%s/app/includes/%s.h"},
		{"main.tpl", "%s/app/src/main.c"},
		{"makefile.tpl", "%s/Makefile"},
		{"readme.tpl", "%s/README.md"},
		{NULL, NULL}
	};
	
	for (int i = 0; templates[i].tpl_file; i++) {
		char tpl_path[512];
		char output_path[512];
		
		snprintf(tpl_path, sizeof(tpl_path), "%s/ft_templates/default/%s", 
				exe_dir, templates[i].tpl_file);
		
		// Tratamento especial para o header
		if (strstr(templates[i].output_format, ".h")) {
			snprintf(output_path, sizeof(output_path), templates[i].output_format,
					project_name, project_name);
		} else {
			snprintf(output_path, sizeof(output_path), templates[i].output_format,
					project_name);
		}
		
		if (process_template(tpl_path, output_path, project_name, include_guard) != 0)
			return 1;
	}
	
	return 0;
}

int cmd_new_execute(t_cli_context *ctx)
{
	printf("Executando comando NEW para projeto %s\n", ctx->project_name);
	
	// Obter caminho do executável
	char exe_path[512];
	ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
	if (len == -1) {
		fprintf(stderr, "Erro ao obter caminho do executável\n");
		return 1;
	}
	exe_path[len] = '\0';
	char *exe_dir = dirname(exe_path);
	
	// Preparar nomes
	char *project_upper = str_to_upper(ctx->project_name);
	if (!project_upper)
		return 1;
	
	char include_guard[256];
	snprintf(include_guard, sizeof(include_guard), "%s_H", project_upper);
	free(project_upper);
	
	// Criar diretórios principais
	const char *dirs[] = {
		"app",
		"app/includes",
		"app/src",
		"app/obj",
		"app/tests",
		"app/bin",
		"doc",
		"doc/assets",
		NULL
	};
	
	fs_create_dir(ctx->project_name);
	for (int i = 0; dirs[i]; i++) {
		create_project_dir(ctx->project_name, dirs[i]);
	}
	
	// Processar templates
	if (process_templates(exe_dir, ctx->project_name, include_guard) != 0)
		return 1;
	
	printf("Estrutura criada com sucesso em %s!\n", ctx->project_name);
	return 0;
}
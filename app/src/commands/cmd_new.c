#include "../../include/ft_cli.h"
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

// Função para criar um diretório
static int create_directory(const char *path)
{
	struct stat st;
	if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
		return 0;
	return mkdir(path, 0775);
}

// Função para ler um arquivo inteiro
static char *read_file(const char *path)
{
	FILE *file = fopen(path, "r");
	if (!file)
		return NULL;
	
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char *content = malloc(size + 1);
	if (!content) {
		fclose(file);
		return NULL;
	}
	
	fread(content, 1, size, file);
	content[size] = '\0';
	fclose(file);
	return content;
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

// Função para processar e escrever um template
static int process_template(const char *template_path, const char *output_path, 
							const char *project_name, const char *include_guard)
{
	char *content = read_file(template_path);
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
	
	// Construir caminhos dos templates
	char include_tpl[512], main_tpl[512], makefile_tpl[512], readme_tpl[512];
	snprintf(include_tpl, sizeof(include_tpl), "%s/templates/default/include.tpl", exe_dir);
	snprintf(main_tpl, sizeof(main_tpl), "%s/templates/default/main.tpl", exe_dir);
	snprintf(makefile_tpl, sizeof(makefile_tpl), "%s/templates/default/makefile.tpl", exe_dir);
	snprintf(readme_tpl, sizeof(readme_tpl), "%s/templates/default/readme.tpl", exe_dir);
	
	// Preparar nomes
	char *project_upper = str_to_upper(ctx->project_name);
	if (!project_upper)
		return 1;
	
	char include_guard[256];
	snprintf(include_guard, sizeof(include_guard), "%s_H", project_upper);
	free(project_upper);
	
	// Criar diretórios
	create_directory(ctx->project_name);
	create_directory("app");
	
	char buf[512];
	snprintf(buf, sizeof(buf), "%s/app", ctx->project_name);
	create_directory(buf);
	
	snprintf(buf, sizeof(buf), "%s/app/includes", ctx->project_name);
	create_directory(buf);
	
	snprintf(buf, sizeof(buf), "%s/app/src", ctx->project_name);
	create_directory(buf);
	
	snprintf(buf, sizeof(buf), "%s/app/obj", ctx->project_name);
	create_directory(buf);
	
	snprintf(buf, sizeof(buf), "%s/app/tests", ctx->project_name);
	create_directory(buf);
	
	snprintf(buf, sizeof(buf), "%s/app/bin", ctx->project_name);
	create_directory(buf);
	
	snprintf(buf, sizeof(buf), "%s/doc", ctx->project_name);
	create_directory(buf);
	
	snprintf(buf, sizeof(buf), "%s/doc/assets", ctx->project_name);
	create_directory(buf);
	
	// Processar templates
	snprintf(buf, sizeof(buf), "%s/app/includes/%s.h", ctx->project_name, ctx->project_name);
	if (process_template(include_tpl, buf, ctx->project_name, include_guard) != 0)
		return 1;
	
	snprintf(buf, sizeof(buf), "%s/app/src/main.c", ctx->project_name);
	if (process_template(main_tpl, buf, ctx->project_name, include_guard) != 0)
		return 1;
	
	snprintf(buf, sizeof(buf), "%s/Makefile", ctx->project_name);
	if (process_template(makefile_tpl, buf, ctx->project_name, include_guard) != 0)
		return 1;
	
	snprintf(buf, sizeof(buf), "%s/README.md", ctx->project_name);
	if (process_template(readme_tpl, buf, ctx->project_name, include_guard) != 0)
		return 1;
	
	printf("Estrutura criada com sucesso em %s!\n", ctx->project_name);
	return 0;
}
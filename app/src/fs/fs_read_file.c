#include <fs.h>

// Função para ler um arquivo inteiro
char *fs_read_file(const char *path)
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
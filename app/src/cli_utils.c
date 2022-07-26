
#include "ft_cli.h"

char	*join(char *s1, char *s2)
{
	char	*join;

	join = malloc(strlen(s1) + strlen(s2) + 1);
	if (join)
	{
		strcpy(join, s1);
		strcat(join, s2);
	}
	return (join);
}


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

char	*strmapi(char const *s, int (*f)(int))
{
	unsigned int	i;
	char			*s_cpy;

	if (!s)
		return (NULL);
	i = 0;
	s_cpy = strdup(s);
	if (!s_cpy)
		return (NULL);
	while (s_cpy[i])
	{
		s_cpy[i] = (*f)(s_cpy[i]);
		i++;
	}
	return (s_cpy);
}

#include "libft.h"

int ft_isnumber(const char *str)
{
	int	len;
	int	i;

	if (str == NULL)
		return (0);
	len = ft_strlen(str);
	i = 0;
	while (i < len)
	{
		if (!ft_isdigit(str[i]))
			return (0);
		i++;
	}
	return (1);
}

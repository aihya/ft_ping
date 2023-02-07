#include "ft_ping.h"

int	ft_strlen(const char *str)
{
	int	size;

	size = 0;
	while (*str && ++str && ++size)
		;
	return (size);
}

void	*ft_memset(void *s, int c, size_t n)
{
	unsigned char	*buff;
	size_t			i;

	buff = (unsigned char *)s;
	i = 0;
	while (i < n)
	{
		buff[i] = (unsigned char)c;
		i++;
	}
	return (s);
}

int	ft_strcmp(const char *s1, const char *s2)
{
	const char	*s1_cpy;
	const char	*s2_cpy;

	s1_cpy = s1;
	s2_cpy = s2;
	while ((*s1_cpy || *s2_cpy) && (*s1_cpy++ == *s2_cpy++))
		;
	return ((unsigned char)*(s1_cpy - 1) - (unsigned char)*(s2_cpy - 1));
}

void	*ft_memcpy(void *dst, const void *src, size_t n)
{
	unsigned char	*udst;
	unsigned char	*usrc;
	int				i;

	udst = (unsigned char *)dst;
	usrc = (unsigned char *)src;
	i = 0;
	while (i < n)
	{
		udst[i] = usrc[i];
		i++;
	}
	return (dst);
}
#include "libft.h"

char	*ft_nbr_base(unsigned long long n, unsigned int base, int padding)
{
	int			i;
	static char	buffer[256] = {0};

	ft_memset(buffer, 0, 256);
	if (base)
	{
		i = 0;
		while (1337)
		{
			buffer[i++] = HEX_CHARS[n % base];
			n /= base;
			if (!n)
				break ;
		}
		while (i < 256 && i < padding)
			buffer[i++] = '0';
		buffer[i] = '\0';
		ft_strrev(buffer);
		return (buffer);
	}
	return (NULL);
}

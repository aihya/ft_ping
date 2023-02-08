#include "libft.h"

void	ft_putnbr_base(unsigned long long n, unsigned int base, int padding)
{
	char	*buf;

	buf = ft_nbr_base(n, base, padding);
	ft_putstr(buf);
}

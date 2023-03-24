#include "libft.h"
    # include <stdio.h>


long long int   ft_atoll(const char *str)
{
    long long int   value;
    int             sign;
    char            *s;

    s = ft_strtrim(str);
	sign = 1;
	if (*s == '-' && (sign = -1))
		s++;
	else if (s[0] == '+')
		s++;
	while (*s == '0')
		s++;
    value = 0;
    while (ft_isdigit(*s))
    {
        value = (value * 10) + (*s - 48);
        s++;
    }
    return (sign * value);
}
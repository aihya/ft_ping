/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aihya <aihya@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/10/12 16:07:24 by aihya             #+#    #+#             */
/*   Updated: 2021/12/24 22:09:20 by aihya            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_putnbr(long long n)
{
	char	*buf;

	buf = NULL;
	if (n < 0)
	{
		ft_putchar('-');
		if (n == LLONG_MIN)
		{
			n = (n / 10) * (-1);
			ft_putnbr(n);
			ft_putchar(-(LLONG_MIN % 10) + 48);
			return ;
		}
		n *= -1;
	}
	buf = ft_nbr_base(n, 10, 0);
	ft_putstr(buf);
}

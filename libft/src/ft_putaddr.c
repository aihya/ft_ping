/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putaddr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aihya <aihya@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/06/30 12:51:06 by aihya             #+#    #+#             */
/*   Updated: 2021/12/23 19:00:29 by aihya            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_malloc.h"

void	ft_putaddr(void *addr, int prefix, int newline, int padding)
{
	if (prefix)
		ft_putstr("0x");
	ft_putnbr_base((unsigned long long)addr, 16, padding);
	if (newline)
		ft_putchar('\n');
}

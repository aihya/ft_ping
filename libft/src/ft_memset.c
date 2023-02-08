/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memset.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aihya <aihya@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/10/06 16:59:09 by aihya             #+#    #+#             */
/*   Updated: 2021/12/24 16:08:50 by aihya            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memset(void *b, int c, size_t len)
{
	size_t	i;

	if (b != NULL)
	{
		i = 0;
		while (i < len)
		{
			*((unsigned char *)b + i) = (unsigned char)c;
			i++;
		}
	}
	return (b);
}

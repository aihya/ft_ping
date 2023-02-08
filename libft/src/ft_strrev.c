/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strrev.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aihya <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/10/13 20:07:32 by aihya             #+#    #+#             */
/*   Updated: 2018/10/28 15:23:40 by aihya            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_strrev(char *str)
{
	int		i;
	int		len;
	char	byte;

	if (!str)
		return ;
	i = -1;
	len = ft_strlen(str);
	while (++i < --len)
	{
		byte = str[i];
		str[i] = str[len];
		str[len] = byte;
	}
}

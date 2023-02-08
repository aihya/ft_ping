/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_begins_with.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aihya <aihya@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/10/26 15:18:40 by aihya             #+#    #+#             */
/*   Updated: 2022/06/25 11:23:10 by aihya            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_begins_with(const char *haystack, const char *needle)
{
	return (!ft_strncmp(haystack, needle, ft_strlen(needle)));
}

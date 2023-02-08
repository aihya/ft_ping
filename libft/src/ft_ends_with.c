/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ends_with.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aihya <aihya@student.1337.ma>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/25 10:43:57 by aihya             #+#    #+#             */
/*   Updated: 2022/06/25 10:45:43 by aihya            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_ends_with(const char *haystack, const char *needle)
{
	size_t	h_len;
	size_t	n_len;

	h_len = ft_strlen(haystack);
	n_len = ft_strlen(needle);
	if (n_len <= h_len)
		return (!ft_strcmp(haystack + (h_len - n_len), needle));
	return (0);
}

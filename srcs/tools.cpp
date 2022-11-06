/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tools.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: narnaud <narnaud@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/02 11:01:55 by narnaud           #+#    #+#             */
/*   Updated: 2022/11/02 11:14:36 by narnaud          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stddef.h>

void	*ft_memset(void *b, int c, size_t len)
{
	size_t	i;
	unsigned char * b_cpy;

	b_cpy = (unsigned char *)b;
	i = 0;
	while (i < len)
		*(unsigned char *)(b_cpy + i++) = (unsigned char)c;
	return ((void *)b);
}

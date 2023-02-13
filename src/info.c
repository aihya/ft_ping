#include "ft_ping.h"

int resolve_hostname(enum e_dest dest, struct in_addr *sin_addr)
{
	int					ret;
	struct sockaddr_in	sa_in;
	struct sockaddr		*sa;
	socklen_t			sa_len;
	char				*buf;
	size_t				buf_len;

	if (dest == END_POINT)
	{
		sa = g_data.dest.sa;
		sa_len = g_data.dest.ai->ai_addrlen;
		buf = g_data.end_hostname;
		buf_len = sizeof(g_data.end_hostname);
	}
	else if (dest == LAST_POINT)
	{
		sa_in.sin_family = AF_INET;
		inet_pton(AF_INET, g_data.last_presentable, &(sa_in.sin_addr));
		sa = (struct sockaddr *)&sa_in;
		sa_len = sizeof(struct sockaddr_in);
		buf = g_data.last_hostname;
		buf_len = sizeof(g_data.last_hostname);
	}
	ret = getnameinfo(sa, sa_len, buf, buf_len, NULL, 0, 0);
	if (ret < 0)
	{
		set_error_codes(GETNAMEINFO, FUNCTION, ret);
		return (-1);
	}
	return (1);
}

int	presentable_format(struct in_addr *sin_addr, char *buffer, size_t len)
{
	if (inet_ntop(AF_INET, sin_addr, buffer, len) == NULL)
	{
		set_error_codes(INET_NTOP, FUNCTION, errno);
		return (-1);
	}
	return (1);
}

struct addrinfo	*resolve_target(char *target)
{
	struct addrinfo	hints;
	struct addrinfo	*result, *res, *p;
	int	ret;

	hints.ai_flags = 0;
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	if ((ret = getaddrinfo(target, NULL, &hints, &res)))
	{
		printf("getaddrinfo: %s\n", gai_strerror(ret));
		set_error_codes(GETADDRINFO, FUNCTION, ret);
		return NULL;
	}
	for (p = res; p != NULL; p = p->ai_next)
	{
		if (p->ai_family   == AF_INET &&
			p->ai_protocol == IPPROTO_ICMP &&
			p->ai_socktype == SOCK_RAW &&
			p->ai_flags    == 0)
			break;
	}
	if (p == NULL)
		return (NULL);
	result = calloc(1, sizeof(struct addrinfo));
	ft_memcpy(result, p, sizeof(struct addrinfo));
	return (result);
}

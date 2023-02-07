#include "ft_ping.h"

int set_hostname(void)
{
	int ret;
	if ((ret = getnameinfo(g_data.dest.sa, g_data.dest.ai->ai_addrlen, 
					g_data.hostname, sizeof(g_data.hostname), NULL, 0, 0)) < 0)
	{
		set_error_codes(GETNAMEINFO, FUNCTION, ret);
		return (-1);
	}
	return (1);
}

void set_presentable_format(void)
{
	struct sockaddr_in *sa_in;
	sa_in = (struct sockaddr_in *)(g_data.dest.sa);
	inet_ntop(AF_INET, &(sa_in->sin_addr), g_data.presentable, sizeof(g_data.presentable));
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
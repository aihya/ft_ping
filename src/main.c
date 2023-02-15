#include "ft_ping.h"

t_data g_data = {0};

/**
 * @brief Get the checksum of the sent and received packets
 * 
 * @param buffer 
 * @param size 
 * @return uint16_t 
 */
uint16_t	checksum(uint16_t *buffer, size_t size)
{
	size_t		count;
	uint32_t	checksum;

	count = size;
	checksum = 0;
	while (count > 1)
	{
		checksum += *buffer++;
		count -= 2;
	}
	if (count)
		checksum += *(uint8_t *)buffer;
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);
	return (~checksum);
}

/**
 * @brief print program usage
 * 
 * @param __exit 
 * @param exit_code 
 */
void usage(int __exit, int exit_code)
{
	printf("\nUsage\n  ping [options] <destination>\n");
	printf("\nOprionts:\n");
	printf("  <destination>      dns name or ip address\n");
	printf("  -h                 print help and exit\n");
	printf("  -v                 verbose output\n");
	printf("  -n                 no dns name resolution\n");
	printf("  -i <interval>      seconds between sending each packet\n");
	printf("  -s <size>          use <size> as number of data bytes to be sent\n");
	printf("  -t <ttl>           define time to live\n");
	printf("  -c <count>         stop after <count> replies\n");
	if (__exit)
		exit(exit_code);
}

/**
 * @brief parse arguments and store the result
 * in t_options structure
 * 
 * @param nargs 
 * @param args 
 */
void	parse_options(int nargs, char **args)
{
	int	i;

	g_data.opt.t = MAXTTL;
	g_data.opt.i = 1;
	g_data.opt.c = -1;
	i = 0;
	while (i < nargs)
	{
		if (!ft_strcmp("-h", args[i]))
			usage(true, 0);
		else if (!ft_strcmp("-v", args[i]))
			g_data.opt.options |= OPT_v;
		else if (!ft_strcmp("-n", args[i]))
			g_data.opt.options |= OPT_n;
		else if (!ft_strcmp("-t", args[i]))
		{
			if (i+1 < nargs && ft_isnumber(args[i+1]) && ft_atoi(args[i+1]) > 0)
			{
				g_data.opt.options |= OPT_t;
				g_data.opt.t = ft_atoi(args[i+1]);
				i += 2;
				continue;
			}
			printf("Invalid option %s\n", args[i]);
			exit(1);
		}
		else if (!ft_strcmp("-s", args[i]))
		{
			if (i+1 < nargs && ft_isnumber(args[i+1]) && ft_atoi(args[i+1]) > 0)
			{
				g_data.opt.options |= OPT_s;
				g_data.opt.s = ft_atoi(args[i+1]);
				i += 2;
				continue;
			}
			printf("Invalid option %s\n", args[i]);
			exit(1);
		}
		else if (!ft_strcmp("-c", args[i]))
		{
			if (i+1 < nargs && ft_isnumber(args[i+1]) && ft_atoi(args[i+1]) > 0)
			{
				g_data.opt.options |= OPT_c;
				g_data.opt.c = ft_atoi(args[i+1]);
				i += 2;
				continue;
			}
			printf("Invalid option %s\n", args[i]);
			exit(1);
		}
		else if (!ft_strcmp("-i", args[i]))
		{
			if (i+1 < nargs && ft_isnumber(args[i+1]) && ft_atoi(args[i+1]) > 0)
			{
				g_data.opt.options |= OPT_i;
				g_data.opt.i = ft_atoi(args[i+1]);
				i += 2;
				continue;
			}
			printf("Invalid option %s\n", args[i]);
			exit(1);
		}
		else if (args[i][0] == '-')
		{
			printf("Invalid option %s\n", args[i]);
			exit(1);
		}
		else
			g_data.target = args[i];
		i++;
	}
}

/**
 * @brief Get the addrinfo object
 * 
 */
void	get_addrinfo()
{
	struct addrinfo	hints;
	struct addrinfo	*result, *p;
	int				ret;

	hints = (struct addrinfo){0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;

	ret = getaddrinfo(g_data.target, NULL, &hints, &result);
	if (ret < 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
		exit(errno);
	}
	for (p = result; p; p = p->ai_next)
	{
		if (p->ai_family == AF_INET && 
			p->ai_socktype == SOCK_RAW &&
			p->ai_protocol == IPPROTO_ICMP)
			break ;
	}
	g_data.dest.ai = *p;
	g_data.dest.sa = g_data.dest.ai.ai_addr;
	g_data.dest.sin = (struct sockaddr_in *)g_data.dest.sa;
	freeaddrinfo(result);
}

/**
 * @brief Set the presentable buffer
 * 
 * @param addr 
 */
void	set_presentable(struct in_addr addr)
{
	inet_ntop(
		AF_INET, &addr, 
		g_data.presentable.buf, 
		sizeof(g_data.presentable.buf)
	);
}

/**
 * @brief Set the hostname object of the responding source
 * 
 * @param buffer 
 * @param addr_in 
 */
void	set_hostname(struct in_addr addr)
{
	struct sockaddr_in	sa_in;
	struct sockaddr		*sa;

	sa_in.sin_family = AF_INET;
	sa_in.sin_addr = addr;
	sa = (struct sockaddr *)&sa_in;
	getnameinfo(
		sa, sizeof(sa_in), 
		g_data.hostname.buf, 
		sizeof(g_data.hostname.buf), 
		NULL, 0, 0
	);
}

void	create_socket()
{
	struct timeval	tv;
	int				__errno;

	g_data.socket.fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (g_data.socket.fd < 0)
	{
		fprintf(stderr, "socket: %s\n", strerror(errno));
		exit(errno);
	}

	tv = (struct timeval){4, 0};
	setsockopt(g_data.socket.fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	__errno = errno;

	setsockopt(g_data.socket.fd, IPPROTO_IP, IP_TTL, &g_data.opt.t, sizeof(g_data.opt.t));
	__errno = __errno || errno;

	setsockopt(g_data.socket.fd, SOL_IP, IP_RECVERR, &tv, sizeof(tv));
	__errno = __errno || errno;

	if (__errno)
	{
		fprintf(stderr, "setsockopt: %s\n", strerror(errno));
		exit(errno);
	}
}

void	sig_handler(int sig)
{
	if (sig == SIGALRM)
	{
		alarm(g_data.opt.i);
		send_icmp();
	}
	else if (sig == SIGINT || sig == SIGQUIT)
	{
		// TODO: print statistics here
		exit(SIGINT);
	}
}

/////////////////////////////////////////////////////
// Function responsible for receiving a response.
/////////////////////////////////////////////////////

void	setup_msghdr(void)
{
	struct msghdr	*msg;
	struct iovec	*iov;

	iov = &(g_data.packet.iov);
	msg = &(g_data.packet.msg);
	iov->iov_base = g_data.packet.recv;
	iov->iov_len = sizeof(g_data.packet.recv);
	msg->msg_iov = iov;
	msg->msg_iovlen = 1;
	msg->msg_control = g_data.packet.control;
	msg->msg_controllen = sizeof(g_data.packet.control);
}

void	process_response(bool error)
{
	struct sock_extended_err	*e;
	struct cmsghdr				*cmsg;

	if (error)
	{
		e = NULL;
		cmsg = CMSG_FIRSTHDR(&g_data.packet.msg);
		while (cmsg)
		{
			if (cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_RECVERR)
				e = (struct sock_extended_err *)CMSG_DATA(cmsg);
			cmsg = CMSG_NXTHDR(&g_data.packet.msg, cmsg);
		}
		if (e)
		{
			
		}
	}
	else
	{
		
	}
}

void	recv_icmp(void)
{
	size_t		bytes;
	struct ip	*ip;
	struct icmp	*icmp;

	bytes = 0 | recvmsg(g_data.socket.fd, &g_data.packet.msg, 0);
	if (bytes > 0)
	{
		ip = (struct ip *)g_data.packet.revc;
		if (ip->ip_p == IPPROTO_IP && ip->ip_v == IPVERSION)
		{
			icmp = (struct icmp *)(g_data.packet.recv + (ip->ip_hl << 2));
			if (icmp->icmp_type == ICMP_ECHOREPLY &&
				icmp->icmp_code == 0 &&
				icmp->icmp_id == (uint16_t)getpid())
				process_response(true);
		}
	}
	else
	{
		bytes = 0 | recvmsg(g_data.socket.fd, &g_data.packet.msg, MSG_ERRQUEUE);
		process_response(false);
	}
	g_data.is_sent = false;
}

/////////////////////////////////////////////////////
// Function responsible for sending the echo request.
/////////////////////////////////////////////////////

void	setup_send_packet(void)
{
	struct icmp	*icmp;

	icmp = (struct icmp *)g_data.packet.send;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = (uint16_t)getpid();
}

void	update_send_packet(void)
{
	struct icmp	*icmp;

	icmp = g_data.packet.send;
	icmp->icmp_seq = ++g_data.sequence;
	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = checksum((uint16_t *)packet, g_data.packet.send_size);
}

void	send_icmp(void)
{
	update_send_packet();
	sendto(
		g_data.socket.fd, 
		g_data.packet.send, 
		g_data.packet.send_size, 
		0, 
		g_data.dest.sa, 
		sizeof(g_data.dest.ai.ai_addrlen)
	);
	g_data.is_sent = true;
}

void	main_loop()
{
	signal(SIGALRM, sig_handler);
	signal(SIGINT, sig_handler);
	signal(SIGQUIT, sig_handler);

	sig_handler(SIGALRM);
	while (g_data.opt.c)
	{
		if (g_data.is_sent)
			recv_icmp();
		if (g_data.opt.options & OPT_c)
			g_data.opt.c--;
		usleep(10);
	}
}

int main(int argc, char **argv)
{
	parse_options(argc-1, argv+1);
	get_addrinfo();
	set_hostname(g_data.dest.sin->sin_addr);
	create_socket();
	setup_send_packet();
	setup_msghdr();
	return (0);
}
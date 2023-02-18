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
	g_data.opt.s = 56;
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
			if (i+1 < nargs && ft_isnumber(args[i+1]) && ft_atoi(args[i+1]) >= 0)
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
	g_data.packet.send_size = ICMP_HDRLEN + g_data.opt.s;
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
	int				on;
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

	on = 1;
	setsockopt(g_data.socket.fd, SOL_IP, IP_RECVERR, &on, sizeof(on));
	__errno = __errno || errno;

	if (__errno)
	{
		fprintf(stderr, "setsockopt: %s\n", strerror(errno));
		exit(errno);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Functions to print bad response errors
////////////////////////////////////////////////////////////////////////////////

void	setup_icmp_msgs(void)
{
	g_data.emsg._0[0] = "Destination network unreachable";
	g_data.emsg._0[1] = "Destination host unreachable";
	g_data.emsg._0[2] = "Destination protocol unreachable";
	g_data.emsg._0[3] = "Destination port unreachable";
	g_data.emsg._0[4] = "Fragmentation required, and DF flag set";
	g_data.emsg._0[5] = "Source route failed";
	g_data.emsg._0[6] = "Destination network unknown";
	g_data.emsg._0[7] = "Destination host unknown";
	g_data.emsg._0[8] = "Source host isolated";
	g_data.emsg._0[9] = "Network administratively prohibited";
	g_data.emsg._0[10] = "Host administratively prohibited";
	g_data.emsg._0[11] = "Network unreachable for ToS";
	g_data.emsg._0[12] = "Host unreachable for ToS";
	g_data.emsg._0[13] = "Communication administratively prohibited";
	g_data.emsg._0[14] = "Host Precedence Violation";
	g_data.emsg._0[15] = "Precedence cutoff in effect";
	g_data.emsg._11[0] = "Time to live exceeded";
	g_data.emsg._11[1] = "Fragment reassembly time exceeded";
}

static char *set_destination_unreachable(int code)
{
	if (code >= 0 && code <= 15)
		return (g_data.emsg._0[code]);
	return (NULL);
}

static char *set_time_exceeded(int code)
{
	if (code >= 0 && code <= 1)
	    return (g_data.emsg._11[code]);
    return (NULL);
}

// TODO: manage option -v in else
char    *set_packet_error_message(int type, int code)
{
	if (type == 3)
		return (set_destination_unreachable(code));
	else if (type == 11)
	{
		return (set_time_exceeded(code));
	}
    return (NULL);
}

////////////////////////////////////////////////////////////////////////////////
// Function responsible for printing the errrors and valid icmp reply.
////////////////////////////////////////////////////////////////////////////////

double	get_time_diff(struct timeval *stime, struct timeval *rtime)
{
	double	time;

	time = (rtime->tv_sec - stime->tv_sec) * 1000.;
	time += (rtime->tv_usec - stime->tv_usec) / 1000.;
	return (time);
}

void	print_header(void)
{
	int payload_size;
	int total_size;

	payload_size = g_data.packet.send_size - ICMP_HDRLEN;
	total_size = g_data.packet.send_size + IPV4_HDRLEN;
	printf("PING %s (%s) %d(%d) bytes of data.\n",
			g_data.target,
			g_data.presentable.buf,
			payload_size,
			total_size);
}

void	print_error(ssize_t bytes, struct sock_extended_err *error)
{
	struct ip			*ip;
	struct icmp			*icmp;
	struct sockaddr_in	*sin;
	char				*error_msg;
	char				*packet;

	sin = (struct sockaddr_in *)SO_EE_OFFENDER(error);
	icmp = (struct icmp *)(g_data.packet.recv);
	set_presentable(sin->sin_addr);
	if (g_data.opt.options & OPT_n)
		printf("From %s ", g_data.presentable.buf);
	else
	{
		set_hostname(sin->sin_addr);
		printf("From %s (%s) ", g_data.hostname.buf, g_data.presentable.buf);
	}
	error_msg = set_packet_error_message(error->ee_type, error->ee_code);
	printf("icmp_seq=%d %s\n", icmp->icmp_seq, error_msg);
}

void	print_icmp_reply(ssize_t bytes)
{
	struct ip		*ip;
	struct icmp		*icmp;
	char			*packet;
	struct timeval	*stime;
	struct timeval	rtime;

	packet = g_data.packet.recv;
	ip = (struct ip *)packet;
	icmp = (struct icmp *)(packet + (ip->ip_hl << 2));
	set_presentable(ip->ip_src);
	if (g_data.opt.options & OPT_n)
	{
		printf("%ld bytes from %s: icmp_seq=%d ttl=%d",
			bytes - ICMP_HDRLEN - IPV4_HDRLEN,
			g_data.presentable.buf,
			icmp->icmp_seq,
			ip->ip_ttl
		);
	}
	else
	{
		printf("%ld bytes from %s (%s): icmp_seq=%d ttl=%d",
			bytes - IPV4_HDRLEN,
			g_data.hostname.buf,
			g_data.presentable.buf,
			icmp->icmp_seq,
			ip->ip_ttl);
	}
	if (g_data.opt.s >= sizeof(struct timeval))
	{
		gettimeofday(&rtime, 0);
		stime = (struct timeval *)(icmp + 1);
		printf(" time=%.1f ms", get_time_diff(stime, &rtime));
	}
	printf("\n");
}

void	print_response(ssize_t bytes, struct sock_extended_err *error)
{
	if (error)
		print_error(bytes, error);
	else
		print_icmp_reply(bytes);
}

void	print_verbose(void)
{

}

////////////////////////////////////////////////////////////////////////////////
// Function responsible for receiving a response. 
////////////////////////////////////////////////////////////////////////////////

void	setup_msghdr(void)
{
	struct msghdr	*msg;
	struct iovec	*iov;

	iov = &(g_data.packet.iov);
	msg = &(g_data.packet.msg);
	// ft_memset(iov, 0x00, sizeof(struct iovec));
	// ft_memset(msg, 0x00, sizeof(struct msghdr));
	iov->iov_base = g_data.packet.recv;
	iov->iov_len = sizeof(g_data.packet.recv);
	msg->msg_iov = iov;
	msg->msg_iovlen = 1;
	msg->msg_control = g_data.packet.ctrl;
	msg->msg_controllen = sizeof(g_data.packet.ctrl);
}

void	process_response(ssize_t bytes)
{
	struct sock_extended_err	*error;
	struct cmsghdr				*cmsg;

	error = NULL;
	cmsg = CMSG_FIRSTHDR(&g_data.packet.msg);
	while (cmsg)
	{
		if (cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_RECVERR)
			error = (struct sock_extended_err *)CMSG_DATA(cmsg);
		cmsg = CMSG_NXTHDR(&g_data.packet.msg, cmsg);
	}
	print_response(bytes, error);
}

void	recv_icmp(void)
{
	ssize_t			bytes;
	struct ip		*ip;
	struct icmp		*icmp;
	struct timeval	rtime;

	rtime = (struct timeval){0};
	ft_memset(g_data.packet.recv, 0x00, sizeof(g_data.packet.recv));
	ft_memset(g_data.packet.ctrl, 0x00, sizeof(g_data.packet.ctrl));
	bytes = recvmsg(g_data.socket.fd, &(g_data.packet.msg), 0);
	if (bytes > 0)
	{
		printf("allo 1\n");
		ip = (struct ip *)g_data.packet.recv;
		if (ip->ip_p == IPPROTO_ICMP && ip->ip_v == IPVERSION)
		{
			icmp = (struct icmp *)(g_data.packet.recv + (ip->ip_hl << 2));
			if (icmp->icmp_type == ICMP_ECHOREPLY &&
				icmp->icmp_code == 0 &&
				icmp->icmp_id == (uint16_t)getpid())
				process_response(bytes);
		}
		if (g_data.opt.options & OPT_v)
			print_verbose();
	}
	else if (bytes < 0)
	{
		printf("allo 2\n");
		bytes = recvmsg(g_data.socket.fd, &g_data.packet.msg, MSG_ERRQUEUE);
		printf("recv bytes(error): %ld\n", bytes);
		process_response(bytes);
	}
	
	g_data.is_sent = false;
}

////////////////////////////////////////////////////////////////////////////////
// Function responsible for sending the echo request.
////////////////////////////////////////////////////////////////////////////////

void	setup_send_packet(void)
{
	struct icmp	*icmp;

	ft_memset(g_data.packet.send, 0x00, sizeof(g_data.packet.send_size));
	icmp = (struct icmp *)g_data.packet.send;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = (uint16_t)getpid();
	icmp->icmp_seq = ++g_data.sequence;
	if (g_data.opt.s >= sizeof(struct timeval))
		gettimeofday((void *)(icmp + 1), 0);
	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = checksum((uint16_t *)icmp, g_data.packet.send_size);
}

void	send_icmp(void)
{
	setup_send_packet();
	sendto(
		g_data.socket.fd, 
		g_data.packet.send, 
		g_data.packet.send_size, 
		0, 
		g_data.dest.sa, 
		g_data.dest.ai.ai_addrlen
	);
	g_data.is_sent = true;
}

////////////////////////////////////////////////////////////////////////////////

void	sig_handler(int sig)
{
	if (sig == SIGALRM)
		send_icmp();
	else if (sig == SIGINT || sig == SIGQUIT)
	{
		// TODO: print statistics here
		exit(SIGINT);
	}
}

void	main_loop()
{
	signal(SIGALRM, sig_handler);
	signal(SIGINT, sig_handler);
	signal(SIGQUIT, sig_handler);
	setup_icmp_msgs();
	setup_msghdr();
	print_header();
	sig_handler(SIGALRM);
	while (g_data.opt.c)
	{
		if (g_data.is_sent == true)
		{
			alarm(g_data.opt.i);
			recv_icmp();
		}
		if (g_data.opt.options & OPT_c)
			g_data.opt.c--;
	}
}

int main(int argc, char **argv)
{
	parse_options(argc-1, argv+1);
	get_addrinfo();
	set_presentable(g_data.dest.sin->sin_addr);
	set_hostname(g_data.dest.sin->sin_addr);
	create_socket();
	main_loop();
	return (0);
}
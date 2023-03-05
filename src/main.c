#include "ft_ping.h"

t_data g_data = {0};

uint16_t	calc_checksum(uint16_t *buffer, size_t size)
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
		else if (args[i][0] == '-')
		{
			printf("Invalid option %s\n", args[i]);
			exit(1);
		}
		else
			g_data.target = args[i];
		i++;
	}
	g_data.packet.size = ICMP_HDRLEN + g_data.opt.s;
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
		g_data.presentable, 
		sizeof(g_data.presentable)
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
		g_data.hostname, 
		sizeof(g_data.hostname), 
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

void	print_rtt()
{
	double	mdev_time;

	printf("min/avg/max/mdev = %.3lf/%.3lf/%.3lf/%.3lf ms",
		g_data.min_time,
		g_data.sum_time / g_data.received,
		g_data.max_time,
		mdev_time);
}

void	print_header(void)
{
	int payload_size;
	int total_size;

	payload_size = g_data.packet.size - ICMP_HDRLEN;
	total_size = g_data.packet.size + IPV4_HDRLEN;
	printf("PING %s (%s) %d(%d) bytes of data.\n",
			g_data.target,
			g_data.presentable,
			payload_size,
			total_size);
}

void	print_error(ssize_t bytes, struct sock_extended_err *exterr)
{
	struct icmphdr		*icmp;
	struct sockaddr_in  *sin;
	char				*error_msg;

	sin = (struct sockaddr_in *)SO_EE_OFFENDER(exterr);
    icmp = (struct icmphdr *)(g_data.equeue.buff);
    set_presentable(sin->sin_addr);
    if (g_data.opt.options & OPT_n)
        printf("From %s ", g_data.presentable);
    else
    {
        set_hostname(sin->sin_addr);
        printf("From %s (%s) ", g_data.hostname, g_data.presentable);
    }
    error_msg = set_packet_error_message(exterr->ee_type, exterr->ee_code);
    printf("icmp_seq=%d %s\n", icmp->un.echo.sequence, error_msg);
}

void	print_icmp_reply(ssize_t bytes, struct timeval *rtime)
{
    struct iphdr	*ip;
    struct icmphdr	*icmp;
    char			*packet;
    struct timeval	*stime;
	double			time_diff;
	struct in_addr	saddr;

    packet = g_data.nqueue.buff;
    ip = (struct iphdr *)packet;
    icmp = (struct icmphdr *)(packet + (ip->ihl << 2));
	saddr.s_addr = ip->saddr;
	set_presentable(saddr);
	if (g_data.opt.options & OPT_n)
	{
		printf("%lu bytes from %s: icmp_seq=%d ttl=%d",
			bytes - ICMP_HDRLEN - IPV4_HDRLEN,
			g_data.presentable,
			icmp->un.echo.sequence,
			ip->ttl
		);
	}
	else
	{
		printf("%lu bytes from %s (%s): icmp_seq=%d ttl=%d",
			bytes - IPV4_HDRLEN,
			g_data.hostname,
			g_data.presentable,
			icmp->un.echo.sequence,
			ip->ttl);
	}
	if (g_data.opt.s >= sizeof(struct timeval))
	{
		stime = (struct timeval *)(icmp + 1);
		time_diff = get_time_diff(stime, rtime);
		g_data.max_time = time_diff >= g_data.max_time ? time_diff : g_data.max_time;
		g_data.min_time = time_diff <= g_data.min_time ? time_diff : g_data.min_time;
		g_data.sum_time += time_diff;
		printf(" time=%.1f ms", time_diff);
	}
	printf("\n");
}

void	print_current_stats()
{
	printf("\r%ld/%ld packets", g_data.received, g_data.transmitted);
	printf(", %ld%%", (size_t)PACKET_LOSE);
	if (g_data.received)
	{
		printf(", ");
		print_rtt();
	}
	printf("\n");
}

void	print_stats()
{
	printf("\n--- %s ping statistics ---\n", g_data.target);
	printf("%ld packets transmitted, %ld received", g_data.transmitted, g_data.received);
	if (g_data.errors)
		printf(", +%ld errors", g_data.errors);
	printf(", %ld%% packet loss", (size_t)PACKET_LOSE);
	printf(", time %.lfms", g_data.time);
	if (g_data.received)
	{
		printf("\nrtt ");
		print_rtt();
	}
	printf("\n\n");
}

void	print_verbose(void)
{

}

////////////////////////////////////////////////////////////////////////////////
// Function responsible for receiving a response. 
////////////////////////////////////////////////////////////////////////////////

void	reset_queue(t_queue *queue)
{
	if (queue == &g_data.equeue)
		ft_memset(queue->buff, 0x00, ICMP_HDRLEN);
	else
		ft_memset(queue->buff, 0x00, IP_MAXPACKET);
	ft_memset(queue->ctrl, 0x00, IP_MAXPACKET);
	queue->iov = (struct iovec){0};
	queue->msg = (struct msghdr){0};
	queue->iov.iov_base = queue->buff;
	queue->iov.iov_len = IP_MAXPACKET;
	queue->msg.msg_iov = &(queue->iov);
	queue->msg.msg_iovlen = 1;
	queue->msg.msg_control = NULL;
	queue->msg.msg_controllen = 0;
	if (queue == &g_data.equeue)
	{
		queue->msg.msg_control = queue->ctrl;
		queue->msg.msg_controllen = IP_MAXPACKET;
	}
	queue->msg.msg_iov = &(queue->iov);
	queue->msg.msg_flags = 0;
}

ssize_t	attempt_receive(void)
{
	ssize_t	bytes;

	bytes = 0;
	while (true)
	{
		reset_queue(&g_data.equeue);
		reset_queue(&g_data.nqueue);
		bytes = recvmsg(g_data.socket.fd, &(g_data.equeue.msg), MSG_ERRQUEUE);
		printf("1: %ld\n", bytes);
		if (bytes > 0)
			break ;
    	bytes = recvmsg(g_data.socket.fd, &(g_data.nqueue.msg), 0);
		printf("2: %ld\n", bytes);
		if (bytes > 0)
			break ;
		usleep(10);
	}
	return bytes;
}

void	recv_icmp(void)
{
	ssize_t						bytes;
	struct iphdr				*ip;
	struct icmphdr				*icmp;
	struct timeval				rtime;
	struct cmsghdr				*cmsg;
	struct sock_extended_err	*exterr;
	printf("%s\n", __FUNCTION__);

	bytes = attempt_receive();
	exterr = NULL;
	cmsg = CMSG_FIRSTHDR(&g_data.equeue.msg);
	while (cmsg)
	{
		if (cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_RECVERR)
			exterr = (struct sock_extended_err *)CMSG_DATA(cmsg);
		cmsg = CMSG_NXTHDR(&g_data.equeue.msg, cmsg);
	}
	if (exterr)
	{
		printf("Error\n");
		g_data.errors++;
		print_error(bytes, exterr);
		return ;
	}

    ip = (struct iphdr *)g_data.nqueue.buff;
    if (ip->protocol == IPPROTO_ICMP && ip->version == IPVERSION)
    {
        icmp = (struct icmphdr *)(g_data.nqueue.buff + (ip->ihl << 2));
        if (icmp->type == ICMP_ECHOREPLY)
        {
            if (icmp->code == 0 &&
                icmp->un.echo.id == (uint16_t)getpid())
            {
                g_data.received++;
                print_icmp_reply(bytes, &rtime);
            }
        }
    }
    else if (g_data.opt.options & OPT_v)
        print_verbose();
}

////////////////////////////////////////////////////////////////////////////////
// Function responsible for sending the echo request.
////////////////////////////////////////////////////////////////////////////////

void	setup_send_packet(void)
{
	struct icmphdr	*icmp;

	ft_memset(g_data.packet.buff, 0x00, sizeof(g_data.packet.size));
	icmp = (struct icmphdr *)g_data.packet.buff;
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = (uint16_t)getpid();
	icmp->un.echo.sequence = ++g_data.sequence;
	if (g_data.opt.s >= sizeof(struct timeval))
		gettimeofday((void *)(icmp + 1), 0);
	icmp->checksum = 0;
	icmp->checksum = calc_checksum((uint16_t *)icmp, g_data.packet.size);
}

void	send_icmp(void)
{
	int	ret;

	printf("%s\n", __FUNCTION__);
	setup_send_packet();
	ret = sendto(
		g_data.socket.fd, 
		g_data.packet.buff, 
		g_data.packet.size, 
		MSG_DONTWAIT, 
		g_data.dest.sa, 
		g_data.dest.ai.ai_addrlen
	);
	if (ret)
		g_data.transmitted++;
}

////////////////////////////////////////////////////////////////////////////////

void	sig_handler(int sig)
{
	if (sig == SIGALRM)
	{
		send_icmp();
		g_data.is_sent = true;
		// g_data.current_time = (struct timeval){0};
		// gettimeofday(&g_data.current_time, 0);
		// g_data.time += get_time_diff(&g_data.start_time, &g_data.current_time);
	}
	else if (sig == SIGINT)
	{
		print_stats();
		exit(SIGINT);
	}
	else if (sig == SIGQUIT)
		print_current_stats();
}

void	main_loop()
{
	signal(SIGALRM, sig_handler);
	signal(SIGINT, sig_handler);
	signal(SIGQUIT, sig_handler);
	setup_icmp_msgs();
	print_header();
	sig_handler(SIGALRM);
	while (g_data.opt.c)
	{
		if (g_data.is_sent == true)
		{
			// g_data.start_time = (struct timeval){0};
			// gettimeofday(&g_data.start_time, 0);
			recv_icmp();
    		alarm(g_data.opt.i);
			g_data.is_sent = false;
		}
		if (g_data.opt.options & OPT_c)
			g_data.opt.c--;
	}
	usleep(10);
	print_stats();
}

int main(int argc, char **argv)
{
	parse_options(argc-1, argv+1);
	get_addrinfo();
	set_presentable(g_data.dest.sin->sin_addr);
	set_hostname(g_data.dest.sin->sin_addr);
	create_socket();
	g_data.transmitted = 0;
	g_data.received = 0;
	g_data.errors = 0;
	g_data.min_time = DBL_MAX;
	g_data.max_time = 0;
	g_data.sum_time = 0;
	g_data.time = 0;
	main_loop();
	return (0);
}

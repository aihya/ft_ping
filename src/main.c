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
			printf("ping: option requires an argument '%s'\n", args[i]);
			usage(true, 1);
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
			printf("ping: option requires an argument '%s'\n", args[i]);
			usage(true, 1);
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
			printf("ping: option requires an argument '%s'\n", args[i]);
			usage(true, 1);
		}
		else if (args[i][0] == '-')
		{
			printf("ping: invalid option -- '%s'\n", args[i]);
			usage(true, 1);
		}
		else
			g_data.target = args[i];
		i++;
	}
	g_data.packet.size = ICMP_HDRLEN + g_data.opt.s;
}


uint16_t	ft_ntohs(uint16_t dword)
{
	if (__BYTE_ORDER == __LITTLE_ENDIAN)
		return (dword << 8) | (dword >> 8);
	return dword;
}


void	get_addrinfo()
{
	struct addrinfo	hints, *p;
	int				ret;

	hints = (struct addrinfo){0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;

	ret = getaddrinfo(g_data.target, NULL, &hints, &g_data.dest.result);
	if (ret < 0)
	{
		fprintf(stderr, "ping: %s: %s\n", g_data.target, gai_strerror(ret));
		exit(errno);
	}
	for (p = g_data.dest.result; p; p = p->ai_next)
	{
		if (p->ai_family == AF_INET && 
			p->ai_socktype == SOCK_RAW &&
			p->ai_protocol == IPPROTO_ICMP)
			break ;
	}
	ft_memcpy(&g_data.dest.ai, p, sizeof(g_data.dest.ai));
	g_data.dest.sa = g_data.dest.ai.ai_addr;
	g_data.dest.sin = (struct sockaddr_in *)(g_data.dest.sa);
}


char	*set_presentable(struct in_addr addr)
{
	ft_memset(g_data.presentable, 0x00, sizeof(g_data.presentable));
	inet_ntop(AF_INET, &addr, g_data.presentable, sizeof(g_data.presentable));
	return g_data.presentable;
}


void	set_hostname(struct in_addr addr)
{
	struct sockaddr_in	sa_in;
	struct sockaddr		*sa;

	sa_in.sin_family = AF_INET;
	sa_in.sin_addr = addr;
	sa = (struct sockaddr *)&sa_in;
	ft_memset(g_data.hostname, 0x00, sizeof(g_data.hostname));
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
	g_data.emsg._0[0] = "Destination Net Unreachable";
	g_data.emsg._0[1] = "Destination Host Unreachable";
	g_data.emsg._0[2] = "Destination Protocol Unreachable";
	g_data.emsg._0[3] = "Destination Port Unreachable";
	g_data.emsg._0[4] = "Frag needed and DF set (mtu = %u)";
	g_data.emsg._0[5] = "Source Route Failed";
	g_data.emsg._0[6] = "Destination Net Unknown";
	g_data.emsg._0[7] = "Destination Host Unknown";
	g_data.emsg._0[8] = "Source Host Isolated";
	g_data.emsg._0[9] = "Destination Net Prohibited";
	g_data.emsg._0[10] = "Destination Host Prohibited";
	g_data.emsg._0[11] = "Destination Net Unreachable for Type of Service";
	g_data.emsg._0[12] = "Destination Host Unreachable for Type of Service";
	g_data.emsg._0[13] = "Packet filtered";
	g_data.emsg._0[14] = "Precedence Violation";
	g_data.emsg._0[15] = "Precedence Cutoff";
	g_data.emsg._5[0] = "Redirect Network";
	g_data.emsg._5[1] = "Redirect Host";
	g_data.emsg._5[2] = "Redirect Type of Service and Network";
	g_data.emsg._5[3] = "Redirect Type of Service and Host";
	g_data.emsg._11[0] = "Time to live exceeded";
	g_data.emsg._11[1] = "Fragment reassembly time exceeded";
}


void	print_verbose(void)
{
	struct iphdr	*ip;
	struct in_addr	addr;

	ip = (struct iphdr *)g_data.queue.buff;
	printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks            Src          Dst\n");
    printf(" %1x  %1x  %02x %04x %04x", ip->version,
                                        ip->ihl,
                                        ip->tos,
                                    	ip->tot_len,
                                        ip->id);
	printf(" %3x %04x", ip->frag_off >> 13, ip->frag_off << 3 >> 3);
	printf("  %2x  %2x %4x", ip->ttl, ip->protocol, ip->check);

	addr.s_addr = ip->saddr;
	printf(" %s", set_presentable(addr));

	addr.s_addr = ip->daddr;
	printf(" %s\n", set_presentable(addr));
}


static void	pr_error_message(int type, int code, int info)
{
	if (type == ICMP_DEST_UNREACH)
	{
		if (code >= 0 && code <= 15)
		{
			if (code == ICMP_FRAG_NEEDED)
				printf(g_data.emsg._0[code], info);
			else
				printf("%s\n", g_data.emsg._0[code]);
		}
		else
			printf("Dest Unreachable, Bad Code: %d\n", code);
	}
	else if (type == ICMP_SOURCE_QUENCH)
		printf("Source Quench\n");
	else if (type == ICMP_REDIRECT)
	{
		if (code >= 0 && code <= 3)
			printf("%s\n", g_data.emsg._5[code]);
		else
			printf("Redirect, Bad Code: %d\n", code);
	}
	else if (type == ICMP_ECHO)
		printf("Echo Request\n");
	else if (type == ICMP_TIME_EXCEEDED)
	{
		if (code == 0 || code == 1)
			printf("%s\n", g_data.emsg._11[code]);
		else
			printf("Time exceeded, Bad Code: %d\n", code);
	}
	else if (type == ICMP_PARAMETERPROB)
		printf("Parameter Problem: Bad IP header\n");
	else if (type == ICMP_TIMESTAMP)
		printf("Timestamp\n");
	else if (type == ICMP_TIMESTAMPREPLY)
		printf("Timestamp Reply\n");
	else if (type == ICMP_INFO_REQUEST)
		printf("Information Request\n");
	else if (type == ICMP_INFO_REPLY)
		printf("Information Reply\n");
	else if (type == ICMP_ADDRESS)
		printf("Address Mask Request\n");
	else if (type == ICMP_ADDRESSREPLY)
		printf("Address Mask Reply\n");
	else
		printf("Bad ICMP type: %d\n", type);
	if (g_data.opt.options & OPT_v)
		print_verbose();
}

////////////////////////////////////////////////////////////////////////////////
// Function responsible for tracking time.
////////////////////////////////////////////////////////////////////////////////

double	get_time_diff(struct timeval *stime, struct timeval *rtime)
{
	double	time;

	time = (rtime->tv_sec - stime->tv_sec) * 1000.;
	time += (rtime->tv_usec - stime->tv_usec) / 1000.;
	return (time);
}


void	add_time(struct timeval send, struct timeval recv)
{
	t_time	*time;

	time = (t_time *)calloc(1, sizeof(t_time));
	time->send = send;
	time->recv = recv;
	if (g_data.stt.rtt.tail)
	{
		g_data.stt.rtt.tail->next = time;
		g_data.stt.rtt.tail = g_data.stt.rtt.tail->next;
	}
	else
	{
		g_data.stt.rtt.head = time;
		g_data.stt.rtt.tail = g_data.stt.rtt.head;
	}
}

void	calculate_ewma()
{
	double	diff;
	t_time	*rtt;

	rtt = g_data.stt.rtt.tail;
	diff = get_time_diff(&rtt->send, &rtt->recv);
    if (g_data.stt.rtt.ewma == 0)
        g_data.stt.rtt.ewma = diff;
    else
        g_data.stt.rtt.ewma = 0.5 * diff + (1 - 0.5) * g_data.stt.rtt.ewma;
}

void	calculate_mdev()
{
	double	mdev;
	double	mean;

	mdev = 0.0;
	mean = g_data.stt.rtt.sum_time / g_data.stt.pkt.nrecv;
	for (t_time *rtt = g_data.stt.rtt.head; rtt; rtt = rtt->next)
		mdev += sqrt(fabs(mean - get_time_diff(&rtt->send, &rtt->recv)));
	g_data.stt.rtt.mdev = mdev / g_data.stt.pkt.nrecv;
}

////////////////////////////////////////////////////////////////////////////////
// Function responsible for printing the errrors and valid icmp reply.
////////////////////////////////////////////////////////////////////////////////

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


void	print_error()
{
	struct iphdr	*ip, *oip;
	struct icmphdr	*icmp, *oicmp;
	struct in_addr	saddr;

	ip    = (struct iphdr   *)(g_data.queue.buff);
    icmp  = (struct icmphdr *)(g_data.queue.buff + (ip->ihl<<2));
	oip   = (struct iphdr   *)(g_data.queue.buff + (ip->ihl<<2) + sizeof(icmp));
	oicmp = (struct icmphdr *)(g_data.queue.buff + (ip->ihl<<2) + ICMP_HDRLEN + (oip->ihl<<2));

	if (oicmp->un.echo.id != (uint16_t)getpid())
		return ;

	saddr.s_addr = ip->saddr;
    set_presentable(saddr);
    if (g_data.opt.options & OPT_n)
        printf("From %s ", g_data.presentable);
    else
    {
        set_hostname(saddr);
        printf("From %s (%s) ", g_data.hostname, g_data.presentable);
    }
    printf("icmp_seq=%d ", oicmp->un.echo.sequence);
    pr_error_message(icmp->type, icmp->code, ip->frag_off >> 13);
}


void	print_icmp_reply(ssize_t bytes, struct timeval *rtime)
{
    struct iphdr	*ip;
    struct icmphdr	*icmp;
    struct timeval	*stime;
	struct in_addr	saddr;
	double			time_diff;
    char			*packet;

    packet = g_data.queue.buff;
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
	if (ft_ntohs(ip->tot_len) - IPV4_HDRLEN < g_data.packet.size)
		printf(" (truncated)");
	else if ((size_t)g_data.opt.s >= sizeof(struct timeval))
	{
		stime = (struct timeval *)(icmp + 1);
		time_diff = get_time_diff(stime, rtime);
		if (time_diff >= g_data.stt.rtt.max_time)
			g_data.stt.rtt.max_time = time_diff;
		if (time_diff <= g_data.stt.rtt.min_time)
			g_data.stt.rtt.min_time = time_diff;
		g_data.stt.rtt.sum_time += time_diff;
		add_time(*stime, *rtime);
		calculate_mdev();
		calculate_ewma();
		printf(" time=%.1f ms", time_diff);
	}
	printf("\n");
}


void	print_current_stats()
{
	printf("\r%ld/%ld packets", g_data.stt.pkt.nrecv, g_data.stt.pkt.nsent);
	printf(", %ld%%", (size_t)LOSS);
	if (g_data.stt.pkt.nrecv && g_data.stt.rtt.head)
	{
		printf(", min/avg/ewma/max = %.3lf/%.3lf/%.3lf/%.3lf ms",
			g_data.stt.rtt.min_time,
			g_data.stt.rtt.sum_time / g_data.stt.pkt.nrecv,
			g_data.stt.rtt.ewma,
			g_data.stt.rtt.max_time
		);
	}
	printf("\n");
}


void	print_stats()
{
	printf("\n--- %s ping statistics ---\n", g_data.target);
	printf("%ld packets transmitted, %ld received", g_data.stt.pkt.nsent, g_data.stt.pkt.nrecv);
	if (g_data.stt.pkt.nerrs)
		printf(", +%ld errors", g_data.stt.pkt.nerrs);
	printf(", %lu%% packet loss", (size_t)LOSS);
	printf(", time %.lfms", g_data.stt.rtt.total_time);
	if (g_data.stt.pkt.nrecv && g_data.stt.rtt.head)
	{
		printf("\nrtt min/avg/max/mdev = %.3lf/%.3lf/%.3lf/%.3lf ms",
			g_data.stt.rtt.min_time,
			g_data.stt.rtt.sum_time / g_data.stt.pkt.nrecv,
			g_data.stt.rtt.max_time,
			g_data.stt.rtt.mdev
		);
	}
	printf("\n");
}

////////////////////////////////////////////////////////////////////////////////
// Function responsible for receiving a response. 
////////////////////////////////////////////////////////////////////////////////

void	reset_queue(t_queue *queue)
{
	ft_memset(queue->buff, 0x00, IP_MAXPACKET);
	queue->iov = (struct iovec){0};
	queue->msg = (struct msghdr){0};
	queue->iov.iov_base = queue->buff;
	queue->iov.iov_len = IP_MAXPACKET;
	queue->msg.msg_iov = &(queue->iov);
	queue->msg.msg_iovlen = 1;
	queue->msg.msg_flags = 0;
}


void	recv_icmp(void)
{
	ssize_t						bytes;
	struct iphdr				*ip;
	struct icmphdr				*icmp;
	struct timeval				rtime;
	static bool					first_recv = true;

	if (first_recv)
	{
		gettimeofday(&g_data.start_time, 0);
		first_recv = false;
	}

	reset_queue(&g_data.queue);
	do {
		bytes = recvmsg(g_data.socket.fd, &(g_data.queue.msg), 0);
	} while (bytes < 0);
	
	gettimeofday(&rtime, 0);

    ip = (struct iphdr *)g_data.queue.buff;
    if (ip->protocol == IPPROTO_ICMP && ip->version == IPVERSION)
    {
        icmp = (struct icmphdr *)(g_data.queue.buff + (ip->ihl<<2));
        if (icmp->type == ICMP_ECHOREPLY)
        {
            if (icmp->code == 0 && icmp->un.echo.id == (uint16_t)getpid())
            {
				g_data.stt.pkt.nrecv++;
                print_icmp_reply(bytes, &rtime);
            }
        }
		else
		{
			g_data.stt.pkt.nerrs++;
			print_error(bytes);
		}
    }
	else if (g_data.opt.options & OPT_v)
		print_verbose();
	if (g_data.opt.c != -1)
		g_data.opt.c--;
}

////////////////////////////////////////////////////////////////////////////////
// Function responsible for sending the echo request.
////////////////////////////////////////////////////////////////////////////////

void	setup_send_packet(void)
{
	struct icmphdr	*icmp;

	ft_memset(g_data.packet.buff, 0x00, g_data.packet.size);
	icmp = (struct icmphdr *)g_data.packet.buff;
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = (uint16_t)getpid();
	icmp->un.echo.sequence = g_data.sequence;
	if ((size_t)g_data.opt.s >= sizeof(struct timeval))
		gettimeofday((void *)(icmp + 1), 0);
	icmp->checksum = 0;
	icmp->checksum = calc_checksum((uint16_t *)icmp, g_data.packet.size);
}


void	send_icmp(void)
{
	int	ret;

	setup_send_packet();
	ret = sendto(
		g_data.socket.fd, 
		g_data.packet.buff, 
		g_data.packet.size, 
		0, 
		g_data.dest.sa, 
		g_data.dest.ai.ai_addrlen
	);
	if (ret)
	{
		g_data.stt.pkt.nsent++;
		g_data.sequence++;
	}
}

////////////////////////////////////////////////////////////////////////////////

void	sig_handler(int sig)
{
	static bool	first_send = true;

	if (sig == SIGALRM)
	{
		send_icmp();
		if (!first_send)
		{
			gettimeofday(&g_data.current_time, 0);
			g_data.stt.rtt.total_time += get_time_diff(
				&g_data.start_time, 
				&g_data.current_time
			);
			gettimeofday(&g_data.start_time, 0);
		}
		first_send = false;
		alarm(g_data.opt.i);
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
	g_data.start_time = (struct timeval){0};
	g_data.current_time = (struct timeval){0};

	sig_handler(SIGALRM);
	while (g_data.opt.c)
	{
		recv_icmp();
		usleep(10);
	}

	print_stats();
}

int main(int argc, char **argv)
{
	parse_options(argc-1, argv+1);
	get_addrinfo();
	set_presentable(g_data.dest.sin->sin_addr);
	set_hostname(g_data.dest.sin->sin_addr);
	create_socket();

	g_data.sequence = 1;
	g_data.stt.pkt.nsent = 0;
	g_data.stt.pkt.nrecv = 0;
	g_data.stt.pkt.nerrs = 0;
	g_data.stt.rtt.min_time = DBL_MAX;
	g_data.stt.rtt.max_time = 0;
	g_data.stt.rtt.sum_time = 0;
	g_data.stt.rtt.ewma = 0;
	g_data.stt.rtt.mdev = 0;
	g_data.stt.rtt.total_time = 0;
	
	main_loop();
	return (0);
}

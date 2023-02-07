#include "ft_ping.h"


int ft_strlen(const char * str)
{
	int size;

	size = 0;
	while (*str && ++str && ++size);
	return size;
}


void *ft_memset(void *s, int c, size_t n)
{
	unsigned char *buff;
	size_t i;

	buff = (unsigned char *)s;
	i = 0;
	while (i < n)
	{
		buff[i] = (unsigned char)c;
		i++;
	}
	return s;
}


int		ft_strcmp(const char *s1, const char *s2)
{
	const char *s1_cpy;
	const char *s2_cpy;

	s1_cpy = s1;
	s2_cpy = s2;
	while ((*s1_cpy || *s2_cpy) && (*s1_cpy++ == *s2_cpy++))
		;
	return ((unsigned char)*(s1_cpy - 1) - (unsigned char)*(s2_cpy - 1));
}


void *ft_memcpy(void *dst, const void *src, size_t n)
{
	unsigned char *udst;
	unsigned char *usrc;
	int i;

	udst = (unsigned char *)dst;
	usrc = (unsigned char *)src;
	i = 0;
	while (i < n)
	{
		udst[i] = usrc[i];
		i++;
	}
	return dst;
}

void set_error_codes(enum e_function function, enum e_error_type type, enum e_error error)
{
	g_data.function = function;
	g_data.error = error;
}


uint16_t calculate_checksum(uint16_t *buffer, size_t size)
{
	size_t      count;
	uint32_t    checksum;

	count = size;
	checksum = 0;

	// Sum all consecutive 16 bits of the buffer.
	while (count > 1)
	{
		checksum += *buffer++;
		count -= 2;
	}

	// Add the last 8 bits, if there's one left.
	if (count)
		checksum += *(uint8_t *)buffer;

	// Add High 8 bits to Low 8 bits of checksum and add the carry bits.
	checksum  = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	// Return the 1's compliment of the checksum.
	return (~checksum);
}


int send_icmp_packet()
{
	static int sequence = 0;
	char packet[ICMP_HDRLEN + 56];
	struct icmp *icmp;
	struct sockaddr *destaddr;
	socklen_t addrlen;
	int ret;

	// Reset the packet to 0
	ft_memset(packet, 0x00, sizeof(packet));

	// Construct ICMP packet
	icmp = (struct icmp *)packet;
	icmp->icmp_code  = 0;
	icmp->icmp_type  = ICMP_ECHO;
	icmp->icmp_seq   = ++sequence;
	icmp->icmp_id    = getpid();

	// Calculate the ICMP checksum
	icmp->icmp_cksum = calculate_checksum((uint16_t *)packet, sizeof(packet));
	g_data.checksum = icmp->icmp_cksum;

	// Store the current time before the packet is sent
	gettimeofday(&(g_data.send_time), 0);

	// Send the packet
	destaddr = g_data.dest.sa;
	addrlen  = g_data.dest.ai->ai_addrlen;
	ret = sendto(g_data.sock_fd, packet, sizeof(packet), 0, destaddr, addrlen);
	if (ret < 0)
	{
		perror("sendto");
		set_error_codes(SENDTO, FUNCTION, 0);
		return (-1);
	}
	g_data.sent = 1;
	return (1);
}


void set_destination_unreachable(int code)
{
	switch (code)
	{
		case 0:
			g_data.packet_error = ICMP_T0_C0;
			break ;
		case 1:
			g_data.packet_error = ICMP_T0_C1;
			break ;
		case 2:
			g_data.packet_error = ICMP_T0_C2;
			break ;
		case 3:
			g_data.packet_error = ICMP_T0_C3;
			break ;
		case 4:
			g_data.packet_error = ICMP_T0_C4;
			break ;
		case 5:
			g_data.packet_error = ICMP_T0_C5;
			break ;
		case 6:
			g_data.packet_error = ICMP_T0_C6;
			break ;
		case 7:
			g_data.packet_error = ICMP_T0_C7;
			break ;
		case 8:
			g_data.packet_error = ICMP_T0_C8;
			break ;
		case 9:
			g_data.packet_error = ICMP_T0_C9;
			break ;
		case 10:
			g_data.packet_error = ICMP_T0_C10;
			break ;
		case 11:
			g_data.packet_error = ICMP_T0_C11;
			break ;
		case 12:
			g_data.packet_error = ICMP_T0_C12;
			break ;
		case 13:
			g_data.packet_error = ICMP_T0_C13;
			break ;
		case 14:
			g_data.packet_error = ICMP_T0_C14;
			break ;
		case 15:
			g_data.packet_error = ICMP_T0_C15;
			break ;
	}
}


void set_time_exceeded(int code)
{
	
}


void set_packet_error_message(int type, int code)
{
	switch (type)
	{
		case 0:
			set_destination_unreachable(code);
			break;
		case 11:
			set_time_exceeded(code);
			break;
		default:
			// TODO: Must manage the -v option here.
			break;
	}
}


int receive_icmp_packet()
{
	char packet[IP_MAXPACKET];
	struct ip *ip;
	struct icmp *icmp;
	struct iovec iov;
	struct msghdr msg;
	struct timeval current_time;
	int icmp_len;
	int ret;
	uint16_t checksum;
	double time;

	ft_memset(packet, 0x00, sizeof(packet));

	// Construct iov structure
	iov.iov_base = packet;
	iov.iov_len = sizeof(packet);

	// Construct msghdr structure
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	// Start receiving a response
	while ((ret = recvmsg(g_data.sock_fd, &msg, 0)) > 0)
	{
	
		if (ret < 0)
		{
			set_error_codes(RECVMSG, FUNCTION, 0);
			return (-1);
		}

		ip = (struct ip *)packet;
		if (ip->ip_p == IPPROTO_ICMP && ip->ip_v == IPVERSION)
		{
			icmp = (struct icmp *)(packet + (ip->ip_hl << 2));
			checksum = calculate_checksum((uint16_t *)icmp, sizeof(packet) - IPV4_HDRLEN);
			if (icmp->icmp_type == ICMP_ECHOREPLY && icmp->icmp_code == 0 && icmp->icmp_id == getpid() && checksum == 0)
				break;
		}
	}

	gettimeofday(&current_time, 0);
	time  = (current_time.tv_sec  - g_data.send_time.tv_sec)  * 1000.;
	time += (current_time.tv_usec - g_data.send_time.tv_usec) / 1000.;

	if (!ft_strcmp(g_data.target, g_data.presentable))
	{
		printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.1lf ms\n", 
			ret - IPV4_HDRLEN,
			g_data.presentable, 
			icmp->icmp_seq, 
			ip->ip_ttl, 
			time
		);
	}
	else
	{
		printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%.1lf ms\n", 
			ret - IPV4_HDRLEN,
			g_data.hostname, 
			g_data.presentable, 
			icmp->icmp_seq, 
			ip->ip_ttl, 
			time
		);
	}
	g_data.sent = 0;
	return (0);
}

int set_hostname()
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


void set_presentable_format()
{
	struct sockaddr_in *sa_in;

	sa_in = (struct sockaddr_in *)(g_data.dest.sa);
	inet_ntop(AF_INET, &(sa_in->sin_addr), g_data.presentable, sizeof(g_data.presentable));
}


struct addrinfo *resolve_target(char *target)
{
	struct addrinfo hints;
	struct addrinfo *result, *res, *p;
	int ret;

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
	return result;
}


void signal_handler(int sig)
{
	if (sig == SIGALRM)
	{
		send_icmp_packet();
		alarm(1);
	}
	else if (sig == SIGINT || sig == SIGQUIT)
	{
		printf("Signal: %s\n", sig == SIGINT ? "SIGINT" : "SIGQUIT");
		g_data.sent = 0;

		// TODO: Print statistics here.

		exit(0);
	}
}


void loop()
{
	signal(SIGALRM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);

	signal_handler(SIGALRM);
	while (true)
	{
		if (g_data.sent)
		{
			receive_icmp_packet();
		}
		usleep(10);
	}
}


int setup_socket()
{
	g_data.sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	return g_data.sock_fd;
}


int main(int argc, char **argv)
{
	g_data.dest.ai = resolve_target(argv[1]);
	if (g_data.dest.ai == NULL)
	{
		// TODO: Print error message here.
		return (1);
	}

	g_data.dest.sa = g_data.dest.ai->ai_addr;

	if (set_hostname() < 0)
	{
		// TODO: Print error message here.
		return (1);
	}

	set_presentable_format();
	if (setup_socket() < 0)
	{
		perror("socket");
		exit(1);
	}

	g_data.target = argv[1];
	loop();
	return (0);
}


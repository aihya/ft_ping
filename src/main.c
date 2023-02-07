#include "ft_ping.h"
#include <stdbool.h>
#define IP4_HDRLEN 20
#define ICMP_HDRLEN 8

int g_sent = 0;

void recv_v4();
void send_v4();

t_proto proto_v4 = {
	NULL,
	recv_v4,
	send_v4,
	NULL,
	NULL,
	-1
};

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

char *reverse_dns_lookup()
{
	int ret;
	char buff[4096];
	char *name;

	ret = getnameinfo(proto_v4.dst_sa, proto_v4.dst_ai->ai_addrlen, buff, sizeof(buff), NULL, 0, 0);
	name = malloc(ft_strlen(buff));
	memset(name, 0, ft_strlen(buff));
	strncpy(name, buff, ft_strlen(buff));
	return (name);
}

void send_v4()
{
	char packet[ICMP_HDRLEN + 56];
	struct icmp *icmp;
	static int seq = 1;

	ft_memset(packet, 0x00, sizeof(packet));

	// Setup the ICMP structure
	icmp = (struct icmp *)(packet);
	icmp->icmp_id    = getpid();
	icmp->icmp_type  = ICMP_ECHO;
	icmp->icmp_code  = 0;
	icmp->icmp_seq   = seq++;

	// Setup data section
	gettimeofday((struct timeval *)(&(icmp->icmp_data)), NULL);
	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = calculate_checksum((uint16_t *)packet, ICMP_HDRLEN + 56);

	printf("%d\n", icmp->icmp_cksum);

	// Send full packet
	int ret = sendto(proto_v4.sockfd, packet, ICMP_HDRLEN + 56, 0, proto_v4.dst_sa, proto_v4.dst_ai->ai_addrlen);
	if (ret < 0)
		printf("sendto failed with error code: %d\n", ret);
	else
		printf("sent %d bytes successfully\n", ret);
    g_sent = 1;
}

void recv_v4()
{
	char			host_name[256];
	char			response[256];
	int				received;
	struct ip		*ip;
	struct icmp		*icmp;
	struct iovec	iov;
	struct msghdr	msghdr;
	struct timeval	tvcurr;
	struct timeval	tvrecv;
	double 			time;

	ft_memset(response, 0x00, 256);

	iov.iov_base = response;
	iov.iov_len  = sizeof(response);

	msghdr.msg_iov        = &iov;
	msghdr.msg_iovlen     = 1;
	msghdr.msg_flags      = 0;
	msghdr.msg_control    = NULL;
	msghdr.msg_controllen = 0;
	msghdr.msg_name       = NULL;
	msghdr.msg_namelen    = 0;

	received = recvmsg(proto_v4.sockfd, &msghdr, 0);
	if (received < 0)
		perror("recvmsg");
	else
		printf("received: %d bytes\n", received);

	ip = (struct ip *)(response);
	if (ip->ip_p != IPPROTO_ICMP)
	{
		g_sent = 0;
		return;
	}

	icmp = (struct icmp *)(response + (ip->ip_hl << 2));
	printf("%d\n", ip->ip_hl << 2);

	if (icmp->icmp_type == ICMP_ECHOREPLY)
	{
		// Message shown on success
		inet_ntop(
			proto_v4.dst_ai->ai_family,
			&((struct sockaddr_in *)proto_v4.dst_sa)->sin_addr,
			host_name,
			256
		);
		tvrecv = *(struct timeval *)(icmp->icmp_data);

		gettimeofday(&tvcurr, NULL);
		time  = (tvcurr.tv_sec  - tvrecv.tv_sec)  * 1000;
		time += (tvcurr.tv_usec - tvrecv.tv_usec) / 1000.0;
		printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%.1lf ms\n", 64, reverse_dns_lookup(), host_name, icmp->icmp_seq, ip->ip_ttl, time);
	}
	else
	{
		printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%.1lf ms\n", 64, reverse_dns_lookup(), host_name, icmp->icmp_seq, ip->ip_ttl, time);
	}
    g_sent = 0;
}

uint16_t calculate_checksum(uint16_t *buff, ssize_t size)
{
	int count = size;
	uint32_t checksum = 0;

	while (count > 1)
	{
		checksum += *(buff++);
		count -= 2;
	}

	if (count)
		checksum += *(uint8_t *)buff;

	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	return (~checksum);
}

int resolve_destination(char *target, struct addrinfo **ai_ptr)
{
	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *ptr;
	int ret;

	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_flags = 0;
	ret = getaddrinfo(target, NULL, &hints, &result);
	if (ret)
		return (ret);
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		if (ptr->ai_family == AF_INET &&
			ptr->ai_protocol == IPPROTO_ICMP && 
			ptr->ai_socktype == SOCK_RAW)
			break;
	}
	*ai_ptr = ptr;
	return (0);
}

int gai_error(char *exe, char *dest, int error)
{
	fprintf(stderr, "%s: %s: %s\n", exe, dest, gai_strerror(error));
	return (1);
}

int socket_setup()
{
	int sockfd;
	int size = 256;

	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd == -1)
		return (sockfd);

	// setsockopt could be used to set the timeout option or the socket buffer size.

	struct timeval trcv;
	trcv.tv_sec = 5;
	trcv.tv_usec = 0;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&trcv, sizeof(struct timeval)) < 0)
	{
		perror("SO_RCVTIMEO");
		exit(1);
	}
	return (sockfd);
}

void sig_handler(int sig)
{
	proto_v4.func_send();
	g_sent = 1;
}

void loop()
{
	// Setup alarm
	signal(SIGALRM, sig_handler);

	// sig_handler(SIGALRM);
	proto_v4.func_send();
	while (true)
	{
		if (g_sent)
		{
			proto_v4.func_recv();
			alarm(1);
		}
		usleep(10);
	}
}

int main(int argc, char **argv)
{
	int error;
	struct addrinfo *ai;
	struct addrinfo src_ai;
	struct in_addr src_in_addr;

	if (argc == 1)
	{
		fprintf(stderr, "%s: usage error: Destination address required\n", argv[0]);
		return (1);
	}

	error = resolve_destination(argv[1], &ai);
	if (error)
		return (gai_error(argv[0], argv[1], error));

	if (ai)
	{
		char buf[256];
		memset(buf, 0, 256);
		inet_ntop(ai->ai_family, &((struct sockaddr_in *)ai->ai_addr)->sin_addr, buf, 256);
		printf("PING %s (%s) 56(84) data bytes.\n", argv[1], buf);
	}

	proto_v4.dst_ai = ai;
	proto_v4.dst_sa = proto_v4.dst_ai->ai_addr;

	int sockfd = socket_setup();
	proto_v4.sockfd = sockfd;

	loop();

	return (0);
}
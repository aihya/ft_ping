#include "ft_ping.h"
#include <stdbool.h>

#define IP4_HDRLEN 20

int g_sent = 0;

// void init_v4(void);
void recv_v4();
void send_v4();
// int init_v6(void);
// int proc_v6(void);
// int send_v6(void);

t_proto proto_v4 = {
	NULL,
	recv_v4,
	send_v4,
	NULL,
	NULL,
	NULL,
	NULL,
	IPPROTO_ICMP,
	-1
};

// t_proto proto_v6 = {
// 	init_v6, 
// 	proc_v6, 
// 	send_v6, 
// 	IPPROTO_ICMPV6
// };

int ft_strlen(const char * str)
{
	int size;

	size = 0;
	while (*str && ++str && ++size);
	return size;
}

char *reverse_dns_lookup()
{
	int ret;
	char buff[256];
	char *name;

	ret = getnameinfo(proto_v4.dst_sa, proto_v4.dst_ai->ai_addrlen, buff, sizeof(buff), NULL, 0, 0);
	name = malloc(ft_strlen(buff) + 1);
	memset(name, 0, ft_strlen(buff) + 1);
	strncpy(name, buff, ft_strlen(buff));
	return (name);
}

void send_v4()
{
	char buff[64];
	struct icmp *icmp;
	static int seq = 1;

	// Setup the ICMP packet header
	icmp = (struct icmp *)(buff);
	icmp->icmp_id    = getpid();
	icmp->icmp_type  = ICMP_ECHO;
	icmp->icmp_code  = 0;
	icmp->icmp_seq   = seq++;
	memset(icmp->icmp_data, 0x00, 56);

	// Setup data section
	gettimeofday((struct timeval *)(icmp->icmp_data), NULL);

	icmp->icmp_cksum = calculate_checksum((uint16_t *)(buff), 64);
	// Send full packet
	int ret = sendto(proto_v4.sockfd, buff, sizeof(buff), 0, proto_v4.dst_sa, proto_v4.dst_ai->ai_addrlen);
	if (ret == -1)
		printf("sendto failed with error code: %d\n", ret);
}

void recv_v4()
{
	char buff[256];
	char controlbuff[256];
	ssize_t received;
	struct iovec iov;
	struct msghdr msghdr;
	struct ip *ip;
	struct icmp *icmp;
	struct timeval tvcurr;
	struct timeval tvrecv;
	double time;

	memset(buff, 0x00, 256);

	iov.iov_base = buff;
	iov.iov_len = sizeof(buff);


	msghdr.msg_iov = &iov;
	msghdr.msg_iovlen = 1;

	msghdr.msg_name = proto_v4.dst_sa;
	msghdr.msg_namelen = proto_v4.dst_ai->ai_addrlen;

	received = recvmsg(proto_v4.sockfd, &msghdr, 0);
	if (received == -1)
		perror("recvmsg");

	ip = (struct ip *)(iov.iov_base);
	icmp = (struct icmp *)(iov.iov_base + (ip->ip_hl << 2));

	// Message shown on success
	char buf[256];
	inet_ntop(
		proto_v4.dst_ai->ai_family,
		&((struct sockaddr_in *)proto_v4.dst_sa)->sin_addr,
		buf,
		256
	);

	tvrecv = *(struct timeval *)(icmp->icmp_data);

	gettimeofday(&tvcurr, NULL);
	time  = (tvcurr.tv_sec  - tvrecv.tv_sec)  * 1000;
	time += (tvcurr.tv_usec - tvrecv.tv_usec) / 1000.0;
	printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%.1lf ms\n", 64, reverse_dns_lookup(), buf, icmp->icmp_seq, ip->ip_ttl, time);
	g_sent = 0;
}

uint16_t calculate_checksum(uint16_t *buff, ssize_t size)
{
	int count = size;
	uint16_t checksum = 0;

	while (count > 1)
	{
		checksum += *(buff++);
		count -= 2;
	}

	if (count > 0)
		checksum += *(uint8_t *)buff;

	while (checksum >> 16)
		checksum = (checksum & 0xffff) + (checksum >> 16);

	return (~checksum);
}

int resolve_destination(char *target, struct addrinfo **ai_ptr)
{
	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *ptr;
	int ret;

	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_TCP;
	ret = getaddrinfo(target, "http", &hints, &result);
	if (ret)
		return (ret);
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		if (ptr->ai_family == AF_INET)
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
	trcv.tv_sec = 10;
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
	alarm(1);
}

void loop()
{
	// Trigger signal to send the first request
	proto_v4.func_send();
	g_sent = 1;

	while (true)
	{
		if (g_sent)
		{
			proto_v4.func_recv();
			alarm(1);
		}
	}
}

int main(int argc, char **argv)
{
	int error;
	struct addrinfo *ai;

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
		printf("PING %s (%s) 56(84) data bytes.\n", ai->ai_canonname ? ai->ai_canonname : buf, buf);
	}

	// Setup alarm
	signal(SIGALRM, sig_handler);


	proto_v4.dst_ai = ai;
	proto_v4.dst_sa = proto_v4.dst_ai->ai_addr;


	int sockfd = socket_setup();
	proto_v4.sockfd = sockfd;

	loop();

	return (0);
}

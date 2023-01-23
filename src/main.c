#include "ft_ping.h"
#include <stdbool.h>

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
	while (*str && ++str && ++size)
		;
	return size;
}

char *reverse_dns_lookup()
{
	int ret;
	char buff[256];
	char *name;

	ret = getnameinfo(proto_v4.dst_sa, proto_v4.dst_ai->ai_addrlen, buff, sizeof(buff), NULL, 0, 0);
	printf("-> %d\n", ft_strlen(buff));
	name = malloc(ft_strlen(buff) + 1);
	strncpy(name, buff, ft_strlen(buff));
	return (name);
}

void send_v4()
{
	char buff[64];
	struct icmp *icmp;

	// Setup the ICMP packet header
	icmp = (struct icmp *)buff;
	icmp->icmp_id    = getpid();
	printf("pid: %d\n", icmp->icmp_id);
	icmp->icmp_type  = ICMP_ECHO;
	icmp->icmp_code  = 0;
	icmp->icmp_seq   = 1;
	memset(icmp->icmp_data, 0x00, 56);

	// Setup data section
	gettimeofday((struct timeval *)(icmp->icmp_data), NULL);

	// Send full packet
	int ret = sendto(proto_v4.sockfd, buff, sizeof(buff), 0, proto_v4.dst_sa, sizeof(struct sockaddr));
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

	msghdr.msg_control = controlbuff;
	msghdr.msg_controllen = sizeof(controlbuff);

	received = recvmsg(proto_v4.sockfd, &msghdr, 0);
	if (received == -1)
		perror("recvmsg");

	ip = (struct ip *)(iov.iov_base);
//	if (ip->ip_p == IPPROTO_ICMP)
//		printf("IPPROTO_ICMP\n");

	icmp = (struct icmp *)(iov.iov_base + (ip->ip_hl << 2));
//	if (icmp->icmp_type == ICMP_ECHOREPLY)
//		printf("ICMP_ECHOREPLY %d\n", icmp->icmp_id);

	// Message shown on error
	// printf("From %s (%s) icmp_seq=%d ");


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
//	printf("%ld %ld\n", (tvcurr.tv_usec - tvrecv.tv_usec), (tvcurr.tv_usec - tvrecv.tv_usec) % 1000);
	printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%.2lf ms\n", 64, reverse_dns_lookup(), buf, icmp->icmp_seq, ip->ip_ttl, time);
}

int calculate_checksum(int id, int seq)
{
	return 0xFFFF - (0x8 + 0x0 + id + seq); // [DEBUG]: Change this
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
		{
			printf("%p\n", ptr); // [DEBUG]
			break;
		}
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
	printf("Signal received\n");
	proto_v4.func_send();
}

void loop()
{
	// Trigger signal to send the first request
	alarm(1);

	while (true)
	{
		proto_v4.func_recv();
		alarm(1);
	}
}

int main(int argc, char **argv)
{
	int error;
	struct addrinfo *ai_ptr;

	if (argc == 1)
	{
		fprintf(stderr, "%s: usage error: Destination address required\n", argv[0]);
		return (1);
	}

	error = resolve_destination(argv[1], &ai_ptr);
	if (error)
		return (gai_error(argv[0], argv[1], error));

	if (ai_ptr)
	{
		char buf[256];
		inet_ntop(ai_ptr->ai_family, &((struct sockaddr_in *)ai_ptr->ai_addr)->sin_addr, buf, 256);
		printf("%s\n", buf);
	}

	// Setup alarm
	signal(SIGALRM, sig_handler);

	proto_v4.dst_ai = ai_ptr;
	proto_v4.dst_sa = proto_v4.dst_ai->ai_addr;

	int sockfd = socket_setup();
	proto_v4.sockfd = sockfd;

	loop();

	return (0);
}

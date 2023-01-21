#include "ft_ping.h"

// void init_v4(void);
void proc_v4(int sockfd);
void send_v4(int sockfd);
// int init_v6(void);
// int proc_v6(void);
// int send_v6(void);

t_proto proto_v4 = {
	NULL,
	proc_v4,
	send_v4,
	NULL,
	NULL,
	IPPROTO_ICMP
};

// t_proto proto_v6 = {
// 	init_v6, 
// 	proc_v6, 
// 	send_v6, 
// 	IPPROTO_ICMPV6
// };

void send_v4(int sockfd)
{
	char buff[64];
	struct icmp *icmp;

	// Setup the ICMP packet header
	icmp = (struct icmp *)buff;
	icmp->icmp_id    = 42;
	icmp->icmp_type  = ICMP_ECHO;
	icmp->icmp_cksum = 0x0000;
	icmp->icmp_code  = 0;
	icmp->icmp_seq   = 1;
	memset(icmp->icmp_data, 0xa5, 56);

	// // Setup data section
	// gettimeofday((struct timeval *)(icmp->icmp_data), NULL);

	// Send full packet
	int ret = sendto(sockfd, buff, sizeof(buff), 0, proto_v4.dst_sa, proto_v4.dst_ai->ai_addrlen);
	if (ret == -1)
		printf("sendto failed with error code: %d\n", ret);
}

void proc_v4(int sockfd)
{
	char buff[256];
	ssize_t received;
	struct iovec iov;
	struct msghdr msghdr;
	struct ip *ip;
	struct icmp *icmp;
	struct timeval tvcurr;
	struct timeval tvrecv;
	double time;

	memset(buff, 0xa5, 64);

	iov.iov_base = buff;
	iov.iov_len = 64;

	msghdr.msg_iov = &iov;
	msghdr.msg_iovlen = 1;

	printf("%d\n", sockfd);
	received = recvmsg(sockfd, &msghdr, 0);
	if (received == -1)
		perror("recvmsg");
	gettimeofday(&tvcurr, NULL);

	ip = (struct ip *)(iov.iov_base);
	if (ip->ip_p == IPPROTO_ICMP)
		printf("IPPROTO_ICMP\n");

	icmp = (struct icmp *)(iov.iov_base + (ip->ip_hl));
	if (icmp->icmp_type == ICMP_ECHOREPLY)
		printf("ICMP_ECHOREPLY %d\n", icmp->icmp_id);

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
	time = (tvcurr.tv_sec - tvrecv.tv_sec) / 1000 + (tvcurr.tv_usec - tvrecv.tv_usec) * 1000;
	printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%f\n", 64, buf, buf, icmp->icmp_seq, ip->ip_ttl, time);
}

int calculate_checksum(int id, int seq)
{
	return 0xFFFF - (0x8 + 0x0 + id + seq);
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
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(int)) < 0)
	{
		perror("%s");
		exit(1);
	}

	struct timeval trcv;
	trcv.tv_sec = 1;
	trcv.tv_usec = 0;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&trcv, sizeof(struct timeval)) < 0)
	{
		perror("SO_RCVTIMEO");
		exit(1);
	}
	return (sockfd);
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

	proto_v4.dst_ai = ai_ptr;
	proto_v4.dst_sa = proto_v4.dst_ai->ai_addr;

	int sockfd = socket_setup();
	proto_v4.func_send(sockfd);
	proto_v4.func_proc(sockfd);

	return (0);
}
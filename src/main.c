#include "ft_ping.h"

// void init_v4(void);
void proc_v4(void);
void send_v4(int sockfd);
// int init_v6(void);
// int proc_v6(void);
// int send_v6(void);

t_proto proto_v4 = {
	NULL,
	&proc_v4,
	&send_v4,
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

void proc_v4()
{

}

void send_v4(int sockfd)
{
	char buff[256];
	int checksum;

	// Setup the ICMP packet header
	// checksum = calculate_checksum();
	buff[0] = 0x08;	// type
	buff[1] = 0x00;	// code
	buff[2] = 0x00;	// checksum hh
	buff[3] = 0x00;	// checksum hh
	buff[4] = 0x13;	// id hh
	buff[5] = 0x37;	// id hh
	buff[6] = 0x00;	// seq
	buff[7] = 0x01;	// seq

	int ret = sendto(sockfd, buff, sizeof(buff), 0, proto_v4.dst_sa, sizeof(proto_v4.dst_sa));
	if (ret == -1)
		printf("sendto failed with error code: %d\n", ret);
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
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(int)) == -1)
	{
		perror("%s");
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

	proto_v4.dst_sa = (struct sockaddr *)ai_ptr->ai_addr;

	int sockfd = socket_setup();
	proto_v4.func_send(sockfd);

	return (0);
}
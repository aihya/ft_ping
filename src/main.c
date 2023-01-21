#include "ft_ping.h"

// int init_v4(void);
// int init_v6(void);
// int proc_v4(void);
// int proc_v6(void);
// int send_v4(void);
// int send_v6(void);

// t_proto proto_v4 = {
// 	init_v4, 
// 	proc_v4, 
// 	send_v4, 
// 	IPPROTO_ICMP
// };

// t_proto proto_v6 = {
// 	init_v6, 
// 	proc_v6, 
// 	send_v6, 
// 	IPPROTO_ICMPV6
// };

int calculate_checksum(int type, int checksum, int id, int seq)
{
	return 0xFFFF - (type + checksum + id + seq);
}

int resolve_destination(char *target)
{
	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *ptr;
	int ret;

	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_TCP;
	ret = getaddrinfo(target, "http", &hints, &result);
	if (ret)
		return (1);
	return (0);
}

int gai_error(char *exe, char *dest, int error)
{
	fprintf(stderr, "%s: %s: %s\n", exe, dest, gai_strerror(error));
	return (1);
}

int main(int argc, char **argv)
{
	int error;

	if (argc == 1)
	{
		fprintf(stderr, "%s: usage error: Destination address required\n", argv[0]);
		return (1);
	}

	error = resolve_destination(argv[1]);
	if (error)
		return (gai_error(argv[0], argv[1], error));
	return (0);
}
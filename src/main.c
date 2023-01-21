#include "ft_ping.h"

int init_v4(void);
int init_v6(void);
int proc_v4(void);
int proc_v6(void);
int send_v4(void);
int send_v6(void);

t_proto proto_v4 = {
	init_v4, 
	proc_v4, 
	send_v4, 
	IPPROTO_ICMP
};

t_proto proto_v6 = {
	init_v6, 
	proc_v6, 
	send_v6, 
	IPPROTO_ICMPV6
};

int calculate_checksum(int type, int checksum, int id, int seq)
{
	return 0xFFFF - (type + checksum + id + seq);
}

int main(int argc, char **argv)
{
	struct addrinfo ai;
	struct addrinfo hints;
	struct addrinfo **res;

	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_TCP;
	int ret = getaddrinfo("google.com", "http", &hints, res);

	printf("%d\n", ret);
	return (0);
}
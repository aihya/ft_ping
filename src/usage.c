#include "ft_ping.h"

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
	printf("  -W <timeout>       time to wait for response\n");
	printf("  -c <count>         stop after <count> replies\n");
	if (__exit)
		exit(exit_code);
}

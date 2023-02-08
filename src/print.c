#include "ft_ping.h"

double	get_time_diff(void)
{
	double	time;

	time = (g_data.r_time.tv_sec - g_data.s_time.tv_sec) * 1000.;
	time += (g_data.r_time.tv_usec - g_data.s_time.tv_usec) / 1000.;
	return (time);
}

void	print_header(void)
{
	int payload_size;
	int total_size;

	payload_size = sizeof(g_data.s_packet) - IPV4_HDRLEN -ICMP_HDRLEN;
	total_size = sizeof(g_data.s_packet);
	printf("PING %s (%s) %d(%d) bytes of data.\n",
			g_data.target,
			g_data.presentable,
			payload_size,
			total_size);
}

void	print_response(int bytes_read, struct ip *ip, struct icmp *icmp)
{
	if (!ft_strcmp(g_data.target, g_data.presentable) || g_data.options & OPT_n)
	{
		printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.1lf ms\n",
			bytes_read - ICMP_HDRLEN - IPV4_HDRLEN,
			g_data.presentable,
			icmp->icmp_seq,
			ip->ip_ttl,
			get_time_diff());
	}
	else
	{
		printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%.1lf ms\n",
			bytes_read - (IPV4_HDRLEN + ICMP_HDRLEN),
			g_data.hostname,
			g_data.presentable,
			icmp->icmp_seq,
			ip->ip_ttl,
			get_time_diff());
	}
}

void	print_verbose(void)
{
	
}

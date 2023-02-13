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

	payload_size = sizeof(g_data.s_packet) - ICMP_HDRLEN;
	total_size = sizeof(g_data.s_packet) + IPV4_HDRLEN;
	printf("PING %s (%s) %d(%d) bytes of data.\n",
			g_data.target,
			g_data.end_presentable,
			payload_size,
			total_size);
}

void	print_error(struct sock_extended_err *error)
{
	struct sockaddr_in	*sin;
	char				*error_msg;
	
	sin = (struct sockaddr_in *)SO_EE_OFFENDER(error);
	presentable_format(&(sin->sin_addr), g_data.last_presentable, sizeof(g_data.last_presentable));
	if (g_data.opt.options & OPT_n)
		printf("From %s ", g_data.last_presentable);
	else
	{
		resolve_hostname(LAST_POINT, &(sin->sin_addr));
		printf("From %s (%s) ", g_data.last_hostname, g_data.last_presentable);
	}
	error_msg = set_packet_error_message(error->ee_type, error->ee_code);
	printf("icmp_seq=%d %s\n", g_data.sequence, error_msg);
}

void	print_response(int bytes, char *packet, struct sock_extended_err *e)
{
	struct ip	*ip;
	struct icmp	*icmp;

	if (e)
		return (print_error(e));

	ip = (struct ip *)packet;
	icmp = (struct icmp *)(packet + (ip->ip_hl << 2));
	if (g_data.opt.options & OPT_n)
	{
		printf("%d bytes from %s: icmp_seq=%d",
			bytes - ICMP_HDRLEN - IPV4_HDRLEN,
			g_data.end_presentable,
			icmp->icmp_seq);
	}
	else
	{
		printf("%d bytes from %s (%s): icmp_seq=%d",
			bytes - IPV4_HDRLEN,
			g_data.end_hostname,
			g_data.end_presentable,
			icmp->icmp_seq);
	}
	printf(" ttl=%d time=%.1f\n", ip->ip_ttl, get_time_diff());
}

void	print_verbose(void)
{
	
}

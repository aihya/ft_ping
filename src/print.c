#include "ft_ping.h"

double	get_time_diff(void)
{
	double	time;

	time = (g_data.r_time.tv_sec - g_data.s_time.tv_sec) * 1000.;
	time += (g_data.r_time.tv_usec - g_data.s_time.tv_usec) / 1000.;
	return (time);
}

void	print_response(int bytes_read, struct ip *ip, struct icmp *icmp)
{
	if (!ft_strcmp(g_data.target, g_data.presentable))
	{
		printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.1lf ms\n",
			bytes_read - IPV4_HDRLEN,
			g_data.presentable,
			icmp->icmp_seq,
			ip->ip_ttl,
			get_time_diff());
	}
	else
	{
		printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%.1lf ms\n",
			bytes_read - IPV4_HDRLEN,
			g_data.hostname,
			g_data.presentable,
			icmp->icmp_seq,
			ip->ip_ttl,
			get_time_diff());
	}
}

static void	set_destination_unreachable(int code)
{
}

static void	set_time_exceeded(int code)
{
}

// TODO: manage option -v in else
static void	set_packet_error_message(int type, int code)
{
	if (type == 3)
		set_destination_unreachable(code);
	else if (type == 11)
		set_time_exceeded(code);
	else
	{
		if (g_data.options & OPT_v)
			print_verbose();
	}
}

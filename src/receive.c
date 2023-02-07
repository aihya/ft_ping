#include "ft_ping.h"

static void	setup_msghdr(void)
{
	g_data.iov.iov_base = g_data.r_packet;
	g_data.iov.iov_len = sizeof(g_data.r_packet);
	g_data.msg.msg_name = NULL;
	g_data.msg.msg_namelen = 0;
	g_data.msg.msg_control = NULL;
	g_data.msg.msg_controllen = 0;
	g_data.msg.msg_flags = 0;
	g_data.msg.msg_iov = &g_data.iov;
	g_data.msg.msg_iovlen = 1;
}

static int	is_valid_packet(void)
{
	struct ip	*ip;
	struct icmp	*icmp;
	uint16_t	checksum;

	ip = (struct ip *)g_data.r_packet;
	if (ip->ip_p == IPPROTO_ICMP && ip->ip_v == IPVERSION)
	{
		icmp = (struct icmp *)(g_data.r_packet + (ip->ip_hl << 2));
		checksum = calculate_checksum((uint16_t *)icmp,
				sizeof(g_data.r_packet) - IPV4_HDRLEN);
		if (icmp->icmp_type == ICMP_ECHOREPLY && \
			icmp->icmp_code == 0 && \
			icmp->icmp_id == getpid() && \
			checksum == 0)
			return (1);
	}
	return (0);
}

int	receive_icmp_packet(void)
{
	struct ip		*ip;
	struct icmp		*icmp;
	int				bytes_read;
	uint16_t		checksum;

	ft_memset(g_data.r_packet, 0x00, sizeof(g_data.r_packet));
	bytes_read = 1;
	while (bytes_read > 0)
	{
		bytes_read = recvmsg(g_data.sock_fd, &g_data.msg, 0);
		if (is_valid_packet())
			break ;
	}
	if (bytes_read < 0)
	{
		set_error_codes(RECVMSG, FUNCTION, 0);
		return (-1);
	}
	gettimeofday(&g_data.r_time, 0);
	print_response(bytes_read, ip, icmp);
	g_data.sent = 0;
	return (0);
}

#include "ft_ping.h"

static void	setup_packet(void)
{
	struct icmp	*icmp;

	ft_memset(g_data.s_packet, 0x00, sizeof(g_data.s_packet));
	icmp = (struct icmp *)(g_data.s_packet);
	icmp->icmp_code = 0;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_seq = ++g_data.sequence;
	icmp->icmp_id = getpid();
	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = calculate_checksum(
			(uint16_t *)(g_data.s_packet),
			sizeof(g_data.s_packet));
	g_data.checksum = icmp->icmp_cksum;
}

int	send_icmp_packet(void)
{
	struct sockaddr	*destaddr;
	socklen_t		addrlen;
	int				bytes_sent;

	destaddr = g_data.dest.sa;
	addrlen = g_data.dest.ai->ai_addrlen;
	setup_packet();
	gettimeofday(&(g_data.s_time), 0);
	bytes_sent = sendto(
			g_data.sock_fd,
			g_data.s_packet,
			sizeof(g_data.s_packet),
			0,
			destaddr,
			addrlen);
	g_data.sent = 1;
	ft_memset(g_data.s_packet, 0x00, sizeof(g_data.s_packet));
	if (bytes_sent < 0)
	{
		printf("bytes_sent: %d\n", bytes_sent);
		set_error_codes(SENDTO, FUNCTION, 0);
		return (-1);
	}
	return (1);
}

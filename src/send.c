#include "ft_ping.h"

static void	setup_packet(void)
{
	struct icmp	*icmp;

	ft_memset(g_data.s_packet, 0x00, sizeof(g_data.s_packet));
	icmp = (struct icmp *)(g_data.s_packet);
	icmp->icmp_code = 0;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_seq = ++g_data.sequence;
	icmp->icmp_id = getpid() & 0xffff;
	icmp->icmp_cksum = calculate_checksum(
			(uint16_t *)(g_data.s_packet),
			sizeof(g_data.s_packet));
}

int	send_icmp_packet(void)
{
	int				bytes_sent;

    printf("[DEBUG] %s\n", __FUNCTION__);
	setup_packet();
	bytes_sent = sendto(
			g_data.sock_fd,
			g_data.s_packet,
			sizeof(g_data.s_packet),
			0,
		    g_data.dest.sa,
			g_data.dest.ai->ai_addrlen);
	gettimeofday(&(g_data.s_time), 0);
	g_data.sent = 1;
	if (bytes_sent < 0)
	{
		printf("bytes_sent: %d\n", bytes_sent);
		set_error_codes(SENDTO, FUNCTION, 0);
		return (-1);
	}
    receive_icmp_packet();
    alarm(1);
	return (1);
}

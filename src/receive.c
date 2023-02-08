#include "ft_ping.h"

static void	setup_msghdr(void)
{
	ft_memset(g_data.r_packet, 0x00, sizeof(g_data.r_packet));
	g_data.iov.iov_base = g_data.r_packet;
	g_data.iov.iov_len = sizeof(g_data.r_packet);
	g_data.msg.msg_name = NULL;
	g_data.msg.msg_namelen = 0;
	g_data.msg.msg_control = g_data.control;
	g_data.msg.msg_controllen = sizeof(g_data.control);
	g_data.msg.msg_flags = 0;
	g_data.msg.msg_iov = &(g_data.iov);
	g_data.msg.msg_iovlen = 1;
}

static int	is_valid_packet(void)
{
	struct ip	*ip;
	struct icmp	*icmp;
	uint16_t	checksum;

	ip = (struct ip *)(g_data.r_packet);
	if (ip->ip_p == IPPROTO_ICMP && ip->ip_v == IPVERSION)
	{
		icmp = (struct icmp *)(g_data.r_packet + (ip->ip_hl << 2));
		checksum = calculate_checksum((uint16_t *)icmp,
				sizeof(g_data.r_packet) - IPV4_HDRLEN);
		printf("%d-%d %d-%d %d-%d\n", icmp->icmp_type, ICMP_ECHOREPLY, icmp->icmp_code, 0, icmp->icmp_id, getpid());
		if (icmp->icmp_type == ICMP_ECHOREPLY && \
			icmp->icmp_code == 0 && \
			icmp->icmp_id == getpid() && \
			checksum == 0)
		{
			printf("Received packet is valid\n");
			return (1);
		}
	}
	return (0);
}

int	receive_icmp_packet(void)
{
	struct ip		*ip;
	struct icmp		*icmp;
	struct cmsghdr		*cmsg;
	int				bytes_read;

	setup_msghdr();
	bytes_read = 1;
	while (bytes_read > 0)
	{
		bytes_read = recvmsg(g_data.sock_fd, &(g_data.msg), MSG_ERRQUEUE|MSG_DONTWAIT);
		printf("errno: %d\n", errno);
		perror("recvmsg");
		if (is_valid_packet())
			break ;
	}
	if (bytes_read < 0)
	{
		printf("brr %d\n", bytes_read);
		for (cmsg = CMSG_FIRSTHDR(&g_data.msg); cmsg; cmsg = CMSG_NXTHDR(&g_data.msg, cmsg))
		{
			if (cmsg->cmsg_level == SOL_IP)
				printf("Error type %d %d\n", cmsg->cmsg_type, SOL_IP);
		}
	}
	g_data.sent = 0;
	if (bytes_read < 0)
	{
		set_error_codes(RECVMSG, FUNCTION, 0);
		return (-1);
	}
	gettimeofday(&g_data.r_time, 0);
	ip = (struct ip *)g_data.r_packet;
	icmp = (struct icmp *)(g_data.r_packet + (ip->ip_hl << 2));
	print_response(bytes_read, ip, icmp);
	return (0);
}

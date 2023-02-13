#include "ft_ping.h"

static void	setup_msghdr(void)
{
	ft_memset(g_data.r_packet, 0x00, sizeof(g_data.r_packet));
	ft_memset(g_data.control, 0x00, sizeof(g_data.control));
	g_data.iov.iov_base = g_data.r_packet;
	g_data.iov.iov_len = sizeof(g_data.r_packet);
	g_data.msg.msg_name = g_data.dest.sa;
	g_data.msg.msg_namelen = g_data.dest.ai->ai_addrlen;
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
		icmp = (struct icmp *)(g_data.r_packet + (ip->ip_hl*4));
		printf("%d -- %d\n", icmp->icmp_id, getpid());
		checksum = calculate_checksum((uint16_t *)icmp,
				sizeof(g_data.r_packet) - IPV4_HDRLEN);
		if (icmp->icmp_type == ICMP_ECHOREPLY &&
			icmp->icmp_code == 0)
		{
			printf("Received packet is valid\n");
			return (1);
		}
	}
	return (0);
}

void	receive_icmp_packet(void)
{
	struct ip					*ip;
	struct icmp					*icmp;
	struct cmsghdr				*cmsg;
	struct sock_extended_err	*error;
	int							error_bytes;
	int							normal_bytes;

	setup_msghdr();
	error = NULL;
	normal_bytes = recvmsg(g_data.sock_fd, &(g_data.msg), 0);
	error_bytes = recvmsg(g_data.sock_fd, &(g_data.msg), MSG_ERRQUEUE);
	if (error_bytes > 0)
	{
		printf("bytes error: %d\n", error_bytes);
		cmsg = CMSG_FIRSTHDR(&(g_data.msg));
		while (cmsg)
		{
			if (cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_RECVERR)
            {
				error = (struct sock_extended_err *)CMSG_DATA(cmsg);
            }
			cmsg = CMSG_NXTHDR(&(g_data.msg), cmsg);
		}
		print_response(error_bytes, g_data.r_packet, error);

	}
    else if (normal_bytes > 0)
    {
	    gettimeofday(&(g_data.r_time), 0);
        ip = (struct ip *)g_data.r_packet;
        icmp = (struct icmp *)(g_data.r_packet + (ip->ip_hl << 2));
        printf("[DEBUG] type: %d code: %d - %d\n", icmp->icmp_type, icmp->icmp_code, errno);
		print_response(normal_bytes, g_data.r_packet, error);

    }

	// gettimeofday(&(g_data.r_time), 0);
	// printf("[DEBUG] bytes: %d e: %p\n", bytes, error);
	// printf("\n");
	// print_response(bytes, g_data.r_packet, error);
	// printf("\n");
	// g_data.sent = 0;
}

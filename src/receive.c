#include "ft_ping.h"

static void	setup_msghdr(void)
{
	ft_memset(g_data.r_packet, 0x00, sizeof(g_data.r_packet));
	ft_memset(g_data.control, 0x00, sizeof(g_data.control));
	g_data.iov.iov_base = g_data.r_packet;
	g_data.iov.iov_len = sizeof(g_data.r_packet);
	g_data.msg = (struct msghdr){0};
	g_data.msg.msg_control = g_data.control;
	g_data.msg.msg_controllen = sizeof(g_data.control);
	g_data.msg.msg_flags = 0;
	g_data.msg.msg_iov = &(g_data.iov);
	g_data.msg.msg_iovlen = 1;
}

void	receive_icmp_error(void)
{
	struct cmsghdr				*cmsg;
	struct sock_extended_err	*error;
	int							bytes;

	bytes = 0 | recvmsg(g_data.sock_fd, &(g_data.msg), MSG_ERRQUEUE);
	g_data.sent = false;
	error = NULL;
	cmsg = CMSG_FIRSTHDR(&(g_data.msg));
	while (cmsg)
	{
		if (cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_RECVERR)
		{
			error = (struct sock_extended_err *)(CMSG_DATA(cmsg));
			break ;
		}
		cmsg = CMSG_NXTHDR(&(g_data.msg), cmsg);
	}
	if (error)
		print_error(bytes, error);
	else if (g_data.opt.options & OPT_v)
		printf("Verbose mode message\n");
	return ;
}

void	receive_icmp_packet(void)
{
	struct ip					*ip;
	struct icmp					*icmp;
	int							bytes;

	setup_msghdr();
	bytes = 0 | recvmsg(g_data.sock_fd, &(g_data.msg), 0);
	ip = (struct ip *)(g_data.r_packet);
	if (ip->ip_p == IPPROTO_ICMP && ip->ip_v == IPVERSION)
	{
		icmp = (struct icmp *)(g_data.r_packet + (ip->ip_hl << 2));
		if (icmp->icmp_type == ICMP_ECHOREPLY &&
			icmp->icmp_code == 0 &&
			icmp->icmp_id == (uint16_t)getpid())
		{
			g_data.sent = false;
			gettimeofday(&(g_data.r_time), 0);
			print_response(bytes);
			return ;
		}
	}
	receive_icmp_error();
}

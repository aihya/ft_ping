#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "ft_ping.h"

void sig_handler(int signum)
{
	// Send an ICMP packet every [1 second] (Consider changing the time).
	char			icmp_send_buf[1024];
	const void *	icmp_send_ptr;

	icmp_send_ptr = (const void *)(&g_icmp.icmp_send);
	
	memcpy(
		icmp_send_buf, 
		icmp_send_ptr, 
		sizeof(g_icmp.icmp_send)
	);

	// sendto(
	// 	g_icmp.sockfd, 
	// 	icmp_send_buf, 
	// 	sizeof(g_icmp.icmp_send), 
	// 	0, 
	// 	g_icmp.dst_sa, 
		
	// );
}


uint16_t    icmp_checksum(uint16_t *addr, int len)
{
    int         nleft;
    uint32_t    sum;
    uint16_t    *w;
    uint16_t    answer;

    nleft = len;
    sum = 0;
    w = addr;
    answer = 0;
    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1)
    {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
}


void setup_icmp_send()
{
	g_icmp.icmp_send.icmp_type  = ICMP_ECHO;
	g_icmp.icmp_send.icmp_code  = 0;
	g_icmp.icmp_send.icmp_seq   = 1;
	g_icmp.icmp_send.icmp_id    = getpid();
	g_icmp.icmp_send.icmp_cksum = icmp_checksum((uint16_t *)&g_icmp.icmp_send, sizeof(g_icmp.icmp_send));
}


void setup_dst_ai(const char * name)
{
	int gai_ret;

	bzero(&g_icmp.hints, sizeof(g_icmp.hints));
	g_icmp.hints.ai_socktype = SOCK_RAW;
	g_icmp.hints.ai_family = AF_INET;
	g_icmp.hints.ai_flags = AI_PASSIVE;

	gai_ret = getaddrinfo(name, NULL, &g_icmp.hints, &g_icmp.dst_ai);
	if (gai_ret)
	{
		fprintf(stderr, "%s: %s\n", name, gai_strerror(gai_ret));
		exit(EXIT_FAILURE);
	}
}


void setup_socket()
{
	g_icmp.sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (g_icmp.sockfd == -1)
	{
		fprintf(stderr, "Failed to create raw socket.\n");
		exit(EXIT_FAILURE);
	}
}


struct icmp unravel_ip(struct msghdr *)
{
	
}


int main(int argc, char **argv)
{
	struct msghdr msg;
	struct sockaddr_in  *sin;
	char send_buf[1024];
	char recv_buf[1024];
	char ctrl_buf[1024];
	struct iovec iovec;
	struct ip *ip;
	struct icmp *icmp;

	(void)argc;

	setup_dst_ai(argv[1]);
	setup_socket();
	setup_icmp_send();

	memset(send_buf, 0, sizeof(send_buf)); // Invalid
	memcpy(
		send_buf, 
		&g_icmp.icmp_send, 
		sizeof(struct icmp)
	); // Invalid

	

	ssize_t s_ret = sendto(
		g_icmp.sockfd, 
		send_buf, 
		sizeof(struct icmp) * 2, 
		0, 
		g_icmp.dst_ai->ai_addr, 
		g_icmp.dst_ai->ai_addrlen
	);

	memset(recv_buf, 0, sizeof(recv_buf));
	msg.msg_name = g_icmp.dst_ai->ai_addr;
	msg.msg_namelen = g_icmp.dst_ai->ai_addrlen;
	msg.msg_control = ctrl_buf;
	msg.msg_controllen = sizeof(ctrl_buf);
	iovec.iov_base = recv_buf;
	iovec.iov_len = sizeof(recv_buf);
	msg.msg_iov = &iovec;
	msg.msg_iovlen = 1;


	ssize_t r_ret = recvmsg(
		g_icmp.sockfd, 
		&msg, 
		MSG_WAITALL
	);

	perror("recvmsg");

	ip = (struct ip *)msg.msg_iov->iov_base;
	icmp = (struct icmp *)((void *)ip + (ip->ip_hl << 2));

	printf("%d\n", icmp->icmp_type);

	printf("--> %ld sent - %ld recieved\n", s_ret, r_ret);
	// g_icmp.icmp_recv = (struct icmp)msg.msg_control;
	return (0);
}

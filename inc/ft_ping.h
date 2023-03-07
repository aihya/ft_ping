#ifndef FT_PING_H
# define FT_PING_H

# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <signal.h>
# include <unistd.h>
# include <string.h>
# include <netinet/ip_icmp.h>
# include <netinet/ip.h>
# include <sys/time.h>
# include <stdbool.h>
# include <limits.h>
# include <errno.h>
# include <float.h>
# include <math.h>
# include "libft.h"

# define IPV4_HDRLEN sizeof(struct iphdr)
# define ICMP_HDRLEN sizeof(struct icmphdr)

# define OPT_v 1
# define OPT_h 2
# define OPT_s 4
# define OPT_n 8
# define OPT_t 16
# define OPT_c 32

# define LOSS (100 - ((g_data.stt.pkt.nrecv*1.) / g_data.stt.pkt.nsent) * 100)

typedef struct s_time
{
	struct timeval	send;
	struct timeval	recv;
	struct s_time	*next;
}	t_time;

typedef struct s_dest
{
	struct sockaddr_in	*sin;
	struct sockaddr		*sa;
	struct addrinfo		ai;
	struct addrinfo		*result;
	char			*target;
}	t_dest;

typedef struct s_options
{
	int options;
	int t;
	int s;
	int n;
	int	c;
	int	i;
}	t_options;

typedef struct s_socket
{
	int	fd;
}	t_socket;

typedef struct	s_queue
{
	char			buff[IP_MAXPACKET];
	struct msghdr	msg;
	struct iovec	iov;
}	t_queue;

typedef struct s_packet
{
	char	buff[IP_MAXPACKET];
	size_t	size;
}	t_packet;

typedef struct	s_emsg
{
	char	*_0[16];
	char	*_11[2];
}	t_emsg;

typedef struct	s_stt
{
	struct
	{
		t_time	*head;
		t_time	*tail;
		double	max_time;
		double	min_time;
		double	sum_time;
		double	mdev;
		double	ewma;
		double	total_time;
	} rtt;
	struct
	{
		size_t	nsent;
		size_t	nrecv;
		size_t	nerrs;
	} pkt;
}	t_stt;

typedef struct s_data
{
	t_packet			packet;
	t_socket			socket;
	t_queue				queue;
	t_dest				dest;
	t_emsg				emsg;
	t_options			opt;
	t_stt				stt;
	char				*target;
	char				hostname[256];
	char				presentable[256];
	int					sequence;
	int					is_sent;
	struct timeval		start_time;
	struct timeval		current_time;
}	t_data;

extern t_data	g_data;

#endif

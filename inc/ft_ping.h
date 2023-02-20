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
# include "libft.h"
# include <linux/errqueue.h>

# define IPV4_HDRLEN sizeof(struct iphdr)
# define ICMP_HDRLEN sizeof(struct icmphdr)

# define OPT_v 1
# define OPT_h 2
# define OPT_s 4
# define OPT_n 8
# define OPT_t 16
# define OPT_c 32

# define PACKET_LOSE (100 - ((g_data.received*1.) / g_data.transmitted) * 100)

typedef struct s_time
{
	struct timeval	tv;
	struct s_time	*next;
}	t_time;

typedef struct s_dest
{
	struct sockaddr_in	*sin;
	struct sockaddr		*sa;
	struct addrinfo		ai;
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

typedef struct s_packet
{
	char			recv[IP_MAXPACKET];
	char			send[IP_MAXPACKET];
	char			ctrl[IP_MAXPACKET];
	struct iovec	iov;
	struct msghdr	msg;
	size_t			send_size;
}	t_packet;

typedef struct	s_hostname
{
	char	buf[256];
}	t_hostname;

typedef struct	s_presentable
{
	char	buf[256];
}	t_presentable;

typedef struct	s_emsg
{
	char	*_0[16];
	char	*_11[2];
}	t_emsg;

typedef struct s_data
{
	t_presentable		presentable;
	t_hostname			hostname;
	t_packet			packet;
	t_socket			socket;
	t_dest				dest;
	t_emsg				emsg;
	t_options			opt;
	int					sequence;
	int					is_sent;
	size_t				transmitted;
	size_t				received;
	size_t				errors;
	char				*target;
	struct timeval		start_time;
	struct timeval		current_time;
	double				time;
	double				max_time;
	double				min_time;
	double				sum_time;
}	t_data;


extern t_data	g_data;


// uint16_t calculate_checksum(uint16_t *buffer, size_t size);

// // send.h
// int		send_icmp_packet(void);

// // receive.h
// void	receive_icmp_packet(void);

// // info.c
// struct addrinfo	*resolve_target(char *target);
// int				presentable_format(struct in_addr *sin_addr, char *buffer, size_t len);
// int				resolve_hostname(enum e_dest dest, struct in_addr *sin_addr);

// // errors.c
// char    *set_packet_error_message(int type, int code);
// void	setup_icmp_msgs(void);
// void	set_error_codes(enum e_function function,
// 						enum e_error_type type,
// 						enum e_error error);

// // print.c
// double	get_time_diff(void);
// void	print_response(int bytes);
// void	print_error(int bytes, struct sock_extended_err *error);
// void	print_verbose(void);
// void	print_header(void);

// // socket.h
// int	setup_socket(void);

// // usage.h
// void usage(int __exit, int exit_code);


#endif

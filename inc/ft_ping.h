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
# include "libft.h"
# include <linux/errqueue.h>

# define IPV4_HDRLEN 20
# define ICMP_HDRLEN 8

# define OPT_v 1
# define OPT_h 2
# define OPT_s 4
# define OPT_n 8
# define OPT_t 16

// Reference of functions used
enum e_function
{
	GETADDRINFO = 255,
	GETNAMEINFO,
	SOCKET,
	SETSOCKOPT,
	SENDTO,
	RECVMSG,
	INET_NTOP,
	INET_PTON
};

enum e_error_type
{
	GENERAL = 1,
	INTERNAL,
	FUNCTION
};

// Error codes
enum e_error
{
	BAD_ALLOCATION
};

enum e_dest
{
	END_POINT,
	LAST_POINT
};

typedef struct s_time
{
	struct timeval	tv;
	struct s_time	*next;
}	t_time;

typedef struct s_dest
{
	struct sockaddr	*sa;
	struct addrinfo	*ai;
}	t_dest;

typedef struct s_options
{
	int options;
	int t;
	int s;
	int n;
	int c;
}	t_options;

typedef struct s_data
{
	t_dest				dest;
	int					sock_fd;
	int					sequence;
	int					sent;
	uint16_t			checksum;
	char				*target;
	char				last_hostname[256];
	char				end_hostname[256];
	char				end_presentable[256];
	char				last_presentable[256];
	char				s_packet[ICMP_HDRLEN + 56];
	char				r_packet[IP_MAXPACKET];
	char				*packet_error;
	char				*icmp_type_0[16];
	char				*icmp_type_11[2];
	char				control[4096]; 
	struct timeval		s_time;
	struct timeval		r_time;
	struct msghdr		msg;
	struct iovec		iov;
	t_options			opt;
	enum e_error_type	type;
	enum e_error		error;
	enum e_function		function;
}	t_data;

# ifndef G_DATA
#  define G_DATA
extern t_data	g_data;
# endif

uint16_t calculate_checksum(uint16_t *buffer, size_t size);

// send.h
int		send_icmp_packet(void);

// receive.h
void	receive_icmp_packet(void);

// info.c
struct addrinfo	*resolve_target(char *target);
int				presentable_format(struct in_addr *sin_addr, char *buffer, size_t len);
int				resolve_hostname(enum e_dest dest, struct in_addr *sin_addr);

// errors.c
void	set_packet_error_message(int type, int code);
void	setup_icmp_msgs(void);
void	set_error_codes(enum e_function function,
						enum e_error_type type,
						enum e_error error);

// print.c
double	get_time_diff(void);
void	print_response(int bytes, char *packet, struct sock_extended_err *e);
void	print_verbose(void);
void	print_header(void);

// socket.h
int	setup_socket(void);

// usage.h
void usage(void);


#endif

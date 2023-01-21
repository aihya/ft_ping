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
# include <strings.h> // TODO: remove this line and bzero function in main
# include <netinet/ip_icmp.h>

typedef struct s_proto
{
    void (*func_init) (void);
    void (*func_proc) (void);
    void (*func_send) (void);
    struct sockaddr *src_sa;
    struct sockaddr *dst_sa;
    int ipproto_type;
}   t_proto;

typedef struct s_icmp
{
    struct addrinfo hints;
    struct addrinfo *src_ai;
    struct addrinfo *dst_ai;
    struct sockaddr *src_sa;
    struct sockaddr *dst_sa;
    struct icmp icmp_send;
    struct icmp *icmp_recv;
    int sockfd;
}   t_icmp;

t_icmp g_icmp = {0};

int calculate_checksum(int type, int checksum, int id, int seq);

#endif

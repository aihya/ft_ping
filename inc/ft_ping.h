#ifndef FT_PING_H
# define FT_PING_H

# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <strings.h> // TODO: remove this line and bzero function in main
#include <netinet/ip_icmp.h>


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

uint16_t icmp_checksum();
void setup_socket();
void setup_icmp_send();
void setup_dst_ai(const char * name);

#endif

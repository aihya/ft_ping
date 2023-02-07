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
# include <netinet/ip.h>
# include <sys/time.h>
# include <stdbool.h>
# include <limits.h>

#define IPV4_HDRLEN (sizeof(struct ip))
#define ICMP_HDRLEN (sizeof(struct icmp) - IPV4_HDRLEN)

// Reference of functions used
enum e_function
{
    GETADDRINFO = 255,
    GETNAMEINFO,
    SENDTO,
    RECVMSG,
    CALLOC,
    MALLOC
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

typedef struct s_time
{
    struct timeval tv;
    struct s_time *next;
}   t_time;

typedef struct s_dest
{
    struct sockaddr *sa;
    struct addrinfo *ai;
}   t_dest;

typedef struct s_data
{
    int     sock_fd;
    t_dest  dest;

    // Packet sent or not
    int sent;

    // The time the packet is sent
    struct timeval send_time;

    // Current checksum value
    uint16_t checksum;

    // Hostname from dns lookup
    char hostname[4029];

    // Error management.
    enum e_error_type   type;
    enum e_error        error;
    enum e_function     function;
}   t_data;

t_data g_data;

#endif

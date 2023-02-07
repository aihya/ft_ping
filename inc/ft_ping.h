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

#define IPV4_HDRLEN (sizeof(struct ip))
#define ICMP_HDRLEN (sizeof(struct icmp) - IPV4_HDRLEN)

#define ICMP_T0_C0 	    "Destination network unreachable"
#define ICMP_T0_C1   	"Destination host unreachable"
#define ICMP_T0_C2 	    "Destination protocol unreachable"
#define ICMP_T0_C3   	"Destination port unreachable"
#define ICMP_T0_C4 	    "Fragmentation required, and DF flag set"
#define ICMP_T0_C5 	    "Source route failed"
#define ICMP_T0_C6   	"Destination network unknown"
#define ICMP_T0_C7 	    "Destination host unknown"
#define ICMP_T0_C8  	"Source host isolated"
#define ICMP_T0_C9 	    "Network administratively prohibited"
#define ICMP_T0_C10 	"Host administratively prohibited"
#define ICMP_T0_C11 	"Network unreachable for ToS"
#define ICMP_T0_C12 	"Host unreachable for ToS"
#define ICMP_T0_C13 	"Communication administratively prohibited"
#define ICMP_T0_C14 	"Host Precedence Violation"
#define ICMP_T0_C15 	"Precedence cutoff in effect" 

#define ICMP_T11_C0 	"Time to live exceeded"
#define ICMP_T11_C1 	"Fragment reassembly time exceeded"


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

    char *target;

    // Packet sent or not
    int sent;

    // The time the packet is sent
    struct timeval send_time;

    // Current checksum value
    uint16_t checksum;

    // Hostname from dns lookup
    char hostname[4096];

    // Presentable format
    char presentable[4096];

    // ICMP packet error message
    char packet_error[256];

    // Error management.
    enum e_error_type   type;
    enum e_error        error;
    enum e_function     function;
}   t_data;

t_data g_data;

#endif

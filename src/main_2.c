#include "ft_ping.h"


int ft_strlen(const char * str)
{
	int size;

    size = 0;
	while (*str && ++str && ++size);
	return size;
}


void *ft_memset(void *s, int c, size_t n)
{
	unsigned char *buff;
	size_t i;

	buff = (unsigned char *)s;
	i = 0;
	while (i < n)
	{
		buff[i] = (unsigned char)c;
		i++;
	}
	return s;
}


void *ft_memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *udst;
    unsigned char *usrc;
    int i;

    udst = (unsigned char *)dst;
    usrc = (unsigned char *)src;
    i = 0;
    while (i < n)
    {
        udst[i] = usrc[i];
        i++;
    }
    return dst;
}


uint16_t calculate_checksum(uint16_t *buffer, size_t size)
{
    size_t      count;
    uint32_t    checksum;

    count = size;
    checksum = 0;

    // Sum all consecutive 16 bits of the buffer.
    while (count > 1)
    {
        checksum += *buffer++;
        count -= 2;
    }

    // Add the last 8 bits, if there's one left.
    if (count)
        checksum += *(uint8_t *)buffer;

    // Add High 8 bits to Low 8 bits of checksum and add the carry bits.
    checksum  = (checksum >> 16) + (checksum & 0xffff);
    checksum += (checksum >> 16);

    // Return the 1's compliment of the checksum.
    return (~checksum);
}


int send_icmp_packet()
{
    static int sequence = 0;
    char packet[ICMP_HDRLEN + 56];
    struct icmp *icmp;
    struct sockaddr *destaddr;
    socklen_t addrlen;
    int ret;

    // Reset the packet to 0
    ft_memset(packet, 0x00, sizeof(packet));

    // Construct ICMP packet
    icmp = (struct icmp *)packet;
    icmp->icmp_type  = ICMP_ECHO;
    icmp->icmp_code  = 0;
    icmp->icmp_seq   = ++sequence;
    icmp->icmp_id    = getpid();
    icmp->icmp_cksum = 0;

    // Calculate the ICMP checksum
    icmp->icmp_cksum = calculate_checksum((uint16_t *)packet, sizeof(packet));

    // Store the current time befire the packet is sent
    gettimeofday(&(g_data.send_time), 0);

    // Send the packet
    destaddr = g_data.dest.sa;
    addrlen  = g_data.dest.ai->ai_addrlen;
    ret = sendto(g_data.sock_fd, packet, sizeof(packet), 0, destaddr, addrlen);
    if (ret < 0)
    {
        set_error_codes(SENDTO, FUNCTION, 0);
        return (-1);
    }
    return (1);
}


void receive_icmp_packet()
{
    char packet[IP_MAXPACKET];
    struct ip *ip;
    struct icmp *icmp;
    struct msghdr msg;
    struct iovec iov;
    int ret;

    // Construct iov structure
    iov.iov_base = packet;
    iov.iov_len = sizeof(packet);

    // Construct msghdr structure
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    // Start receiving a response
    ret = recvmsg(g_data.sock_fd, &msg, 0);
    if (ret < 0)
    {
        set_error_codes(RECVMSG, FUNCTION, 0);
        return (-1);
    }

    ip = (struct ip *)packet;
    if (ip->ip_p != IPPROTO_ICMP || ip->ip_v != AF_INET)
    {
        exit(0);
    }
    icmp = (struct icmp *)(packet + (ip->ip_hl << 2));
    
}


void set_error_codes(enum e_function function, enum e_error_type type, enum e_error error)
{
    g_data.function = function;
    g_data.error = error;
}


int get_hostname()
{
    int ret;

    if ((ret = getnameinfo(g_data.dest.sa, g_data.dest.ai->ai_addrlen, 
                    g_data.hostname, sizeof(g_data.hostname), NULL, 0, 0)) < 0)
    {
        set_error_codes(GETNAMEINFO, FUNCTION, ret);
        return (1);
    }

    return (-1);
}


struct addrinfo *resolve_target(char *target)
{
    struct addrinfo hints;
    struct addrinfo *result, *res, *p;
    int ret;

    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_ICMP;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;

    if ((ret = getaddrinfo(target, NULL, &hints, &res)))
    {
        set_error_codes(GETADDRINFO, FUNCTION, ret);
        return NULL;
    }

    for (p = res; p; p = p->ai_next)
    {
        if (p->ai_family == AF_INET &&
            p->ai_protocol == IPPROTO_ICMP &&
            p->ai_socktype == SOCK_RAW)
            break;
    }
    result = calloc(1, sizeof(struct addrinfo));
    ft_memcpy(result, p, sizeof(struct addrinfo));
    return result;
}


void signal_handler(int sig)
{
    if (sig == SIGALRM)
    {
        send_icmp_packet();
        alarm(1);
    }
    else if (sig == SIGINT || sig == SIGQUIT)
    {
        printf("Signal: %s\n", sig == SIGINT ? "SIGINT" : "SIGQUIT");
        g_data.sent = 0;

        // TODO: Print statistics here.

        exit(0);
    }
}


void loop()
{
    signal(SIGALRM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);

    signal_handler(SIGALRM);
    while (true)
    {
       if (g_data.sent)
           receive_icmp_packet();
        usleep(10);
    }
}


int main(int argc, char **argv)
{
    g_data.dest.ai = resolve_target(argv[1]);
    if (g_data.dest.ai == NULL)
    {
        // TODO: Print error message here.
        return (1);
    }

    g_data.dest.sa = g_data.dest.ai->ai_addr;

    if (!get_hostname())
    {
        // TODO: Print error message here.
        return (1);
    }

    printf("[Target]: %s\n", g_data.hostname);
    loop();
    return (0);
}


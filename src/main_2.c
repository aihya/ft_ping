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

void send_packet()
{

}

void receive_packet()
{

}

void set_error_codes(enum e_function function, enum e_error_type type, enum e_error error)
{
    g_data.function = function;
    g_data.error = error;
}

int dns_lookup()
{
    int ret;

    if ((ret = getnameinfo(g_data.dest.sa, g_data.dest.ai->ai_addrlen, 
                    g_data.hostname, sizeof(g_data.hostname), NULL, 0, 0)) < 0)
    {
        set_error_codes(GETNAMEINFO, FUNCTION, ret);
        return (0);
    }

    return (1);
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

int main(int argc, char **argv)
{
    g_data.dest.ai = resolve_target(argv[1]);
    if (g_data.dest.ai == NULL)
    {
        // TODO: Print error message here.
        return -1;
    }

    g_data.dest.sa = g_data.dest.ai->ai_addr;

    if (!dns_lookup())
    {
        // TODO: Print error message here.
        return -1;
    }

    return (0);
}

#include "ft_ping.h"

int	setup_socket()
{
    struct timeval time;

	g_data.sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (g_data.sock_fd < 0)
    {
        set_error_codes(SOCKET, FUNCTION, 0);
        return (-1);
    }
    time.tv_sec = 4;
    time.tv_usec = 0;
    if (setsockopt(g_data.sock_fd, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time)))
    {
        set_error_codes(SETSOCKOPT, FUNCTION, 0);
        return (-1);
    }
	return (g_data.sock_fd);
}

// TODO: Must setup the setsockopt function for timeout
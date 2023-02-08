#include "ft_ping.h"


uint16_t	calculate_checksum(uint16_t *buffer, size_t size)
{
	size_t		count;
	uint32_t	checksum;

	count = size;
	checksum = 0;
	while (count > 1)
	{
		checksum += *buffer++;
		count -= 2;
	}
	if (count)
		checksum += *(uint8_t *)buffer;
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);
	return (~checksum);
}

void	signal_handler(int sig)
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

void	loop()
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

void set_options(int argc, char **argv)
{
	int i;

	i = 1;
	while (i < argc)
	{
		if (!ft_strcmp("-v", argv[i]))
			g_data.options |= OPT_v;
		else if (!ft_strcmp("-h", argv[i]))
			g_data.options |= OPT_h;
		else if (!ft_strcmp("-s", argv[i]))
			g_data.options |= OPT_s;
		else if (!ft_strcmp("-n", argv[i]))
			g_data.options |= OPT_n;
		i++;
	}
}

int main(int argc, char **argv)
{
	set_options(argc, argv);
	g_data.dest.ai = resolve_target(argv[1]);
	if (g_data.dest.ai == NULL)
	{
		// TODO: Print error message here.
		return (1);
	}
	g_data.dest.sa = g_data.dest.ai->ai_addr;
	if (set_hostname() < 0)
	{
		// TODO: Print error message here.
		return (1);
	}
	set_presentable_format();
	if (setup_socket() < 0)
	{
		perror("socket");
		exit(1);
	}
	g_data.target = argv[1];
	loop();
	return (0);
}

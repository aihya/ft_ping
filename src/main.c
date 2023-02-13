#include "ft_ping.h"

t_data g_data = {0};

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

static void	signal_handler(int sig)
{
	if (sig == SIGALRM)
	{
		send_icmp_packet();
		}
	else if (sig == SIGINT || sig == SIGQUIT)
	{
		printf("Signal: %s\n", sig == SIGINT ? "SIGINT" : "SIGQUIT");
		g_data.sent = 0;
		// TODO: Print statistics here.
		exit(0);
	}
}

static void	loop()
{
	signal(SIGALRM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	print_header();
	// signal_handler(SIGALRM);
	send_icmp_packet();
	while (true)
	{
		if (g_data.sent)
		{
			receive_icmp_packet();
			alarm(1);
		}
		usleep(10);
	}
}

static void set_options(int argc, char **argv)
{
	int i;
	int counter;

	g_data.opt.t = MAXTTL;
	counter = 0;
	i = 1;
	while (i < argc)
	{
		if (!ft_strcmp("-v", argv[i]))
			g_data.opt.options |= OPT_v;
		else if (!ft_strcmp("-h", argv[i]))
		{
			usage();
			exit(0);
		}
		else if (!ft_strcmp("-s", argv[i]))
			g_data.opt.options |= OPT_s;
		else if (!ft_strcmp("-t", argv[i]))
		{
			if (i+1 < argc && ft_isnumber(argv[i+1]) && ft_atoi(argv[i+1]) > 0)
			{
				g_data.opt.options |= OPT_t;
				g_data.opt.t = ft_atoi(argv[i+1]);
				i += 2;
				continue;
			}
			printf("Invalid option -t\n");
			exit(1);
		}
		else if (!ft_strcmp("-n", argv[i]))
			g_data.opt.options |= OPT_n;
		else
		{
			g_data.target = argv[i];
			counter++;
		}
		if (counter > 1)
		{
			usage();
			exit(1);
		}
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
	g_data.target = argv[1];
	if (resolve_hostname(END_POINT, NULL) < 0)
	{
		// TODO: Print error message here.
		return (1);
	}
	if (presentable_format(&((struct sockaddr_in *)g_data.dest.sa)->sin_addr,
			g_data.end_presentable,
			sizeof(g_data.end_presentable)) == -1)
	{
		// print error here
		return (-1);
	}
	if (!ft_strcmp(g_data.end_presentable, g_data.target))
			g_data.opt.options |= OPT_n;
	if (setup_socket() < 0)
	{
		perror("socket");
		exit(1);
	}
	setup_icmp_msgs();
	loop();
	return (0);
}

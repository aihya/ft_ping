#include "ft_ping.h"

void	setup_icmp_msgs(void)
{
	g_data.icmp_type_0[0] = "Destination network unreachable";
	g_data.icmp_type_0[1] = "Destination host unreachable";
	g_data.icmp_type_0[2] = "Destination protocol unreachable";
	g_data.icmp_type_0[3] = "Destination port unreachable";
	g_data.icmp_type_0[4] = "Fragmentation required, and DF flag set";
	g_data.icmp_type_0[5] = "Source route failed";
	g_data.icmp_type_0[6] = "Destination network unknown";
	g_data.icmp_type_0[7] = "Destination host unknown";
	g_data.icmp_type_0[8] = "Source host isolated";
	g_data.icmp_type_0[9] = "Network administratively prohibited";
	g_data.icmp_type_0[10] = "Host administratively prohibited";
	g_data.icmp_type_0[11] = "Network unreachable for ToS";
	g_data.icmp_type_0[12] = "Host unreachable for ToS";
	g_data.icmp_type_0[13] = "Communication administratively prohibited";
	g_data.icmp_type_0[14] = "Host Precedence Violation";
	g_data.icmp_type_0[15] = "Precedence cutoff in effect";
	g_data.icmp_type_11[0] = "Time to live exceeded";
	g_data.icmp_type_11[1] = "Fragment reassembly time exceeded";
}

static char *set_destination_unreachable(int code)
{
	if (code >= 0 && code <= 15)
		return (g_data.icmp_type_0[code]);
	return (NULL);
}

static char *set_time_exceeded(int code)
{
	if (code >= 0 && code <= 1)
	    return (g_data.icmp_type_11[code]);
    return (NULL);
}

// TODO: manage option -v in else
char    *set_packet_error_message(int type, int code)
{
	if (type == 3)
		return (set_destination_unreachable(code));
	else if (type == 11)
		return (set_time_exceeded(code));
	else
	{
		if (g_data.opt.options & OPT_v)
			print_verbose();
	}
    return (NULL);
}

// Error management function for internal errors
void	set_error_codes(enum e_function function,
						enum e_error_type type,
						enum e_error error)
{
	g_data.function = function;
	g_data.type = type;
	g_data.error = error;
}

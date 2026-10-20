#include "ft_ping.h"
#include "icmp.h"

int	main(int argc, char **argv)
{
	t_addrinfo	*resolved_addr;

	resolved_addr = NULL;

	if(argc != 2)
	{
		printf("ft_ping: usage error: Destination address required\n");
		return (2);
	}

	if (resolve_hostname(argv[1], &resolved_addr) != 0) // find an IPv4 address associated with hostname
	{
		cleanup(&resolved_addr);
		return (1);
	}

	if (exec_icmp(resolved_addr) != 0)	// exec the whole ICMP stuff
	{
		cleanup(&resolved_addr);
		return(1);
	}



	return (cleanup(&resolved_addr));
}

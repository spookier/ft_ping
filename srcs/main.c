#include "ft_ping.h"
#include "icmp.h"

int	main(int argc, char **argv)
{
	t_addrinfo	*res;


	res = NULL;

	if(argc != 2)
	{
		printf("ft_ping: usage error: Destination address required\n");
		return (2);
	}

	if (resolve_hostname(argv[1], &res) != 0) // find an IPv4 address associated with hostname
	{
		cleanup(&res);
		return (1);
	}

	if (exec_icmp() != 0)	// exec the whole ICMP stuff
	{
		cleanup(&res);
		return(1);
	}



	return (cleanup(&res));
}

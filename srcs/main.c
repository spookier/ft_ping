#include "ft_ping.h"

int	main(int argc, char **argv)
{

	t_addrinfo	*res;

	if(argc != 2)
	{
		printf("ft_ping: usage error: Destination address required\n");
		return (2);
	}

	res = NULL;

	if (resolve_hostname(argv[1], &res) != 0)
	{
		cleanup(&res);
		return (1);
	}


	if (open_icmp_socket() != 0)
	{
		cleanup(&res);
		return(1);
	}



	return (cleanup(&res));
}

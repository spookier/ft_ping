#include "ft_ping.h"

int	main(int argc, char **argv)
{

	if(argc != 2)
	{
		printf("Invalid ping command.\n");
		return(1);
	}

	if (resolve_hostname(argv[1]) != 0)
		return (1);



	return (0);
}

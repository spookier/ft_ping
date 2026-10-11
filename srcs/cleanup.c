#include "ft_ping.h"

int	cleanup(t_addrinfo **res)
{
	if (*res != NULL)
		freeaddrinfo(*res);

	*res = NULL;

	return(0);
}

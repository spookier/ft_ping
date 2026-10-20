#include "ft_ping.h"

int	cleanup(t_addrinfo **resolved_addr)
{
	if (*resolved_addr != NULL)
		freeaddrinfo(*resolved_addr);

	*resolved_addr = NULL;

	return(0);
}

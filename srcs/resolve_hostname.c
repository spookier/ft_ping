#include "ft_ping.h"

int	resolve_hostname(const char *hostname, t_addrinfo **res)
{
	t_addrinfo			hints;
	struct sockaddr_in	*addr;
	char				ip[INET_ADDRSTRLEN];
	const char			*str;
	int					ret;

	*res = NULL;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4 addresses only

	ret = getaddrinfo(hostname, NULL, &hints, res); // finds the IP for hostname, does DNS resolution automatically
	if(ret != 0)
	{
		str = gai_strerror(ret);
		printf("ft_ping: %s: %s\n", hostname, str);
		return(1);
	}

	addr = (struct sockaddr_in *)(*res)->ai_addr; // the cast lets us access that address as IPv4, it does not copy it
	if (inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip)) == NULL) // transforms bytes into printable string
	{
		perror("ft_ping: inet_ntop");
		return (1);
	}

	printf("%s\n", ip); // ip now contains readable text like "127.0.0.1"

	return (0);
}

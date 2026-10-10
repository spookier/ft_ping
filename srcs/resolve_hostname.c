#include "ft_ping.h"

int	resolve_hostname(const char *hostname)      // find an IPv4 address associated with hostname
{

	struct addrinfo hints;
	struct addrinfo *res;
	int ret;
	const char *str;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;

	ret = getaddrinfo(hostname, NULL, &hints, &res); // turns hostname into a struct sockaddr, does DNS resolution automatically
	if(ret != 0)
	{
		str = gai_strerror(ret);
		printf("%s\n", str);
		return(1);
	}

	struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
	char ip[INET_ADDRSTRLEN];

	if (inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip)) == NULL) // transform bytes into printable string
	{
		perror("inet_ntop");
		freeaddrinfo(res);
		return (1);
	}

	printf("%s\n", ip);

	freeaddrinfo(res);

	return (0);
}

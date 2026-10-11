#include "ft_ping.h"

int open_icmp_socket()
{
	int fd;

	fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); // AF_INET=ipv4, SOCK_RAW=manually build this layer
	if (fd == -1)
	{
		perror("socket");
		return (1);
	}
	return(0);
}

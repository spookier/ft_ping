#include "icmp.h"

static int	open_icmp_socket(void)
{
	int	sock_fd;

	sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); // AF_INET= ipv4, SOCK_RAW= manually build this layer
	if (sock_fd == -1)
	{
		perror("ft_ping: socket");
		return (-1);
	}
	return (sock_fd);
}

int	exec_icmp(void)
{
	int	sock_fd;

	sock_fd = open_icmp_socket();
	if (sock_fd == -1)
		return (1);

	close(sock_fd);
	return (0);
}

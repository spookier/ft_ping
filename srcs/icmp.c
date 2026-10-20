#include "icmp.h"

static uint16_t checksum(const void *data, size_t len)
{
	const uint8_t	*buf = data;
	uint32_t		sum;
	size_t			i;

	sum = 0;
	for (i = 0; i + 1 < len; i += 2)
	{
		sum += (buf[i] << 8) | buf[i + 1]; // reads 2 bytes as a 16bit number and add to sum
	}
	if (i < len) // if one byte is left, use it as the high byte of the last word
		sum += buf[i] << 8;

	// add any bits above 16 back into the lower 16 bits
	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	// flip the final 16 bits to produce the checksum
	return ((uint16_t)~sum);
}

static int	open_icmp_socket()
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

static void create_icmp_header(struct icmphdr *icmpHeader) // setup ICMP header
{
	uint16_t identifier;

	memset(icmpHeader, 0, sizeof(*icmpHeader));

	icmpHeader->type = ICMP_ECHO;

	identifier = getpid() & 0xFFFF;
	icmpHeader->un.echo.id = htons(identifier);
	icmpHeader->un.echo.sequence = htons(1);
	icmpHeader->checksum = htons(checksum(icmpHeader, sizeof(*icmpHeader)));
}

static ssize_t	send_icmp(int sock_fd, struct icmphdr *icmpHeader, t_addrinfo *resolved_addr)
{
	ssize_t size;
	size = sendto(sock_fd, icmpHeader, sizeof(*icmpHeader), 0, resolved_addr->ai_addr, resolved_addr->ai_addrlen);
	return(size);
}

int	exec_icmp(t_addrinfo *resolved_addr)
{
	struct icmphdr	icmpHeader;
	int				sock_fd;
	ssize_t			sent;

	sock_fd = open_icmp_socket();
	if (sock_fd == -1)
		return (1);

	create_icmp_header(&icmpHeader);

	sent = send_icmp(sock_fd, &icmpHeader, resolved_addr);
	if(sent == -1)
	{
		perror("ft_ping: sendto");
		close(sock_fd);
		return(1);
	}

	close(sock_fd);
	return (0);
}

#include "icmp.h"

static uint16_t checksum(const void *data, size_t len) // calculates a value used to check if the packet was damaged
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

static int	open_icmp_socket() // opens a raw socket for ICMP packets
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

static void create_icmp_header(struct icmphdr *icmpHeader) // sets up the ICMP header
{
	uint16_t identifier;

	memset(icmpHeader, 0, sizeof(*icmpHeader));

	icmpHeader->type = ICMP_ECHO;

	identifier = getpid() & 0xFFFF;
	icmpHeader->un.echo.id = htons(identifier);
	icmpHeader->un.echo.sequence = htons(1);
	icmpHeader->checksum = htons(checksum(icmpHeader, sizeof(*icmpHeader)));
}

static ssize_t	send_icmp(int sock_fd, struct icmphdr *icmpHeader, t_addrinfo *resolved_addr) // sends the ICMP header to destination
{
	return(sendto(sock_fd, icmpHeader, sizeof(*icmpHeader), 0, resolved_addr->ai_addr, resolved_addr->ai_addrlen));
}

static ssize_t	receive_icmp(int sock_fd) // receives packet
{
	unsigned char		buffer[2048];
	struct sockaddr_in 	sender;
	socklen_t 			sender_len;
	ssize_t				received;

	sender_len = sizeof(sender);
	memset(&sender, 0, sizeof(sender));

	received = recvfrom(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender, &sender_len);
	return(received);
}

int	exec_icmp(t_addrinfo *resolved_addr) // opens the socket, prepares the header, sends it and waits for a packet
{
	struct icmphdr	icmpHeader;
	int				sock_fd;
	ssize_t			received;


	sock_fd = open_icmp_socket();
	if (sock_fd == -1)
		return (1);

	create_icmp_header(&icmpHeader);

	if(send_icmp(sock_fd, &icmpHeader, resolved_addr) == -1)
	{
		perror("ft_ping: sendto");
		close(sock_fd);
		return(1);
	}

	received = receive_icmp(sock_fd);
	if(received == -1)
	{
		perror("ft_ping: recv");
		close(sock_fd);
		return(1);
	}


	close(sock_fd);
	return (0);
}

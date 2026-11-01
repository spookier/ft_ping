#include "icmp.h"

static int	icmp_open_socket() // opens a raw socket for ICMP packets
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

static ssize_t	icmp_send(int sock_fd, struct icmphdr *icmp_header, t_addrinfo *resolved_addr) // sends the ICMP header to destination
{
	return(sendto(sock_fd, icmp_header, sizeof(*icmp_header), 0, resolved_addr->ai_addr, resolved_addr->ai_addrlen));
}

int	icmp_run(t_addrinfo *resolved_addr) // opens the socket, prepares the header, sends it and waits for a packet
{
	struct icmphdr	icmp_header;
	int				sock_fd;
	ssize_t			received;
	t_timespec		sent_at;
	t_timespec		received_at;
	double			rtt_ms;

	sock_fd = icmp_open_socket();
	if (sock_fd == -1)
		return (1);

	icmp_create_header(&icmp_header);

	/* start timing before sending to measure the round trip */
	if(clock_gettime(CLOCK_MONOTONIC, &sent_at) == -1)	 // RTT countdown start
	{
		perror("ft_ping: clock_gettime");
		close(sock_fd);
		return(1);
	}

	if(icmp_send(sock_fd, &icmp_header, resolved_addr) == -1)
	{
		perror("ft_ping: sendto");
		close(sock_fd);
		return(1);
	}

	received = icmp_receive(sock_fd, &icmp_header, resolved_addr, &sent_at);
	if(received == -1)
	{
		close(sock_fd);
		return(1);
	}
	if(received == 0) // no reply arrived before the deadline
	{
		printf("ft_ping: request timed out\n");
		close(sock_fd);
		return(1);
	}

	/* stop timing once we get a reply */
	if(clock_gettime(CLOCK_MONOTONIC, &received_at) == -1)	 // RTT countdown end
	{
		perror("ft_ping: clock_gettime");
		close(sock_fd);
		return(1);
	}

	rtt_ms = (received_at.tv_sec - sent_at.tv_sec) * 1000.0; // convert s to ms
	rtt_ms += (received_at.tv_nsec - sent_at.tv_nsec) / 1000000.0; // convert nanoseconds to ms, add the diff

	printf("%f\n", rtt_ms);
	close(sock_fd);
	return (0);
}

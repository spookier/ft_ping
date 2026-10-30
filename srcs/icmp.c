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

static int	inspect_packet(const unsigned char *buffer, size_t received, const struct icmphdr *icmpHeader) // inspects the received packet
{
	size_t			ipv4_len;
	struct icmphdr 	icmp_reply;

	if (received < 20 || (buffer[0] >> 4) != 4)	// checks if first half-byte is IPv4
		return (-1);

	// [IP HEADER][ICMP HEADER]
	ipv4_len = (buffer[0] & 0x0F) * 4; // we get the last half-byte, * 4 to know end of IPv4 packet

	if (ipv4_len < 20) // an IPv4 header must be at least 20 bytes
		return (-1);

	if (received < ipv4_len + 8) // check if we got the IP header + 8 bytes for ICMP header
		return (-1);

	if(buffer[ipv4_len] == ICMP_ECHOREPLY)
	{
		memcpy(&icmp_reply, &buffer[ipv4_len], sizeof(icmp_reply)); // we transform buffer into a struct for ez access

		if(icmp_reply.code == 0 && icmp_reply.un.echo.id == icmpHeader->un.echo.id && icmp_reply.un.echo.sequence == icmpHeader->un.echo.sequence) // check if reply is same as request
		{
			if(checksum(&buffer[ipv4_len], received - ipv4_len) == 0) // checks for corruption, if ICMP is intact
			{

				return(0);
			}
		}
		return(2);
	}
	else
	{
		// Anything else, ignore
		return(2);
	}
}

static ssize_t	receive_icmp(int sock_fd, const struct icmphdr *icmpHeader, const t_addrinfo *resolved_addr, const t_timespec *sent_at) // receives packet
{
	unsigned char				buffer[2048];
	struct sockaddr_in 			sender;
	socklen_t 					sender_len;
	ssize_t						received;
	int							ret;
	const struct sockaddr_in 	*destination;
	t_timespec 					deadline;
	t_timespec 					now;
	double						remaining_ms;
	int							timeout_ms;
	struct pollfd				poll_fd;


	memset(&sender, 0, sizeof(sender));
	destination = (const struct sockaddr_in *)resolved_addr->ai_addr;

	deadline.tv_sec = sent_at->tv_sec + 2;
	deadline.tv_nsec = sent_at->tv_nsec;

	poll_fd.fd = sock_fd;			// we poll() our socketfd
	poll_fd.events = POLLIN;		// "tell me when there's data to read"

	while(1)		// awaiting packet loop
	{
		sender_len = sizeof(sender);

		if(clock_gettime(CLOCK_MONOTONIC, &now) == -1)	// get time to calculate waiting time
		{
			perror("ft_ping: clock_gettime");
			return(-1);
		}

		remaining_ms = (deadline.tv_sec - now.tv_sec) * 1000.0; // seconds left, converted to ms
		remaining_ms += (deadline.tv_nsec - now.tv_nsec) / 1000000.0; // add the nanosecond difference in ms

		if (remaining_ms <= 0) // deadline reached, stop waiting
			return (0);

		timeout_ms = (int)remaining_ms; // keep the whole milliseconds
		if (timeout_ms < remaining_ms) // round up if there was a fractional part
			timeout_ms++;

		ret = poll(&poll_fd, 1, timeout_ms);
		if(ret == -1)
		{
			printf("ft_ping: poll: %s\n", strerror(errno));
			return (-1);
		}

		if(ret == 0) // timeout, no packet arrived
			return (0);
		if(ret > 0 && (poll_fd.revents & POLLIN) != 0) // data is ready to read
		{
			sender_len = sizeof(sender);
			received = recvfrom(sock_fd, buffer, sizeof(buffer), 0,(struct sockaddr *)&sender, &sender_len);
			if (received == -1)
			{
				perror("ft_ping: recv");
				return (-1);
			}
		}
		else // socket woke up without readable data
		{
			printf("ft_ping: poll: socket is not readable\n");
			return (-1);
		}



		ret = inspect_packet(buffer, (size_t)received, icmpHeader);
		if(ret == 0) // this packet is an answer to my ping
		{
			if(sender.sin_addr.s_addr == destination->sin_addr.s_addr)
			{
				return(received);
			}
		}
		else					// this packet is something else, so reloop
		{
			continue ;
		}
	}
}

int	exec_icmp(t_addrinfo *resolved_addr) // opens the socket, prepares the header, sends it and waits for a packet
{
	struct icmphdr	icmpHeader;
	int				sock_fd;
	ssize_t			received;
	t_timespec		sent_at;
	t_timespec		received_at;
	double			rtt_time;

	sock_fd = open_icmp_socket();
	if (sock_fd == -1)
		return (1);

	create_icmp_header(&icmpHeader);

	if(clock_gettime(CLOCK_MONOTONIC, &sent_at) == -1)	 // RTT countdown start
	{
		perror("ft_ping: clock_gettime");
		close(sock_fd);
		return(1);
	}

	if(send_icmp(sock_fd, &icmpHeader, resolved_addr) == -1)
	{
		perror("ft_ping: sendto");
		close(sock_fd);
		return(1);
	}

	received = receive_icmp(sock_fd, &icmpHeader, resolved_addr, &sent_at);
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

	if(clock_gettime(CLOCK_MONOTONIC, &received_at) == -1)	 // RTT countdown end
	{
		perror("ft_ping: clock_gettime");
		close(sock_fd);
		return(1);
	}

	rtt_time = (received_at.tv_sec - sent_at.tv_sec) * 1000.0; // convert s to ms
	rtt_time += (received_at.tv_nsec - sent_at.tv_nsec) / 1000000.0; // convert nanoseconds to ms, add the diff

	printf("%f\n", rtt_time);
	close(sock_fd);
	return (0);
}

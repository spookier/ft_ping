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

int	icmp_exec(t_addrinfo *resolved_addr) // opens the socket, prepares the header, sends it and waits for a packet
{
	struct icmphdr	icmp_header;
	int				sock_fd;
	ssize_t			received;
	t_timespec		sent_at;
	t_timespec		received_at;
	double			rtt_ms;
	double			elapsed_s;
	t_IcmpProbe 	probes[1024] = {0};
	t_IcmpProbe		*matched_probe = NULL;
	size_t			used_count = 0;
	t_timespec 		deadline;


	sock_fd = icmp_open_socket();
	if (sock_fd == -1)
		return (1);

	for(int i = 1; i <= 4; ++i)
	{
		icmp_create_header(&icmp_header, i);

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

		// save the request we just sent in our storage to compare it with replied one later
		probes[used_count].sequence = ntohs(icmp_header.un.echo.sequence);
		probes[used_count].sent_at = sent_at;
		used_count += 1;

		// temporary diagnostics: time each send relative to the first probe
		elapsed_s = (double)(sent_at.tv_sec - probes[0].sent_at.tv_sec);
		elapsed_s += (sent_at.tv_nsec - probes[0].sent_at.tv_nsec) / 1000000000.0;
		printf("SEND seq=%u elapsed=%.3f s\n",
			(unsigned int)probes[used_count - 1].sequence, elapsed_s);

		if (i < 4) // set the deadline to the next request's scheduled time
		{
			deadline = probes[0].sent_at;
			deadline.tv_sec += i;
		}
		else // set the deadline to two seconds after the last request
		{
			deadline = sent_at;
			deadline.tv_sec += 2;
		}

		while (1) // receive replies until deadline
		{

			received = icmp_receive(sock_fd, &icmp_header, resolved_addr, &deadline, probes, used_count, &matched_probe);
			if(received == -1)
			{
				close(sock_fd);
				return(1);
			}
			if(received == 0) // waiting period ended, advance the sending loop
				break ;

			if(clock_gettime(CLOCK_MONOTONIC, &received_at) == -1)	 // RTT countdown end
			{
				perror("ft_ping: clock_gettime");
				close(sock_fd);
				return(1);
			}

			rtt_ms = (received_at.tv_sec - matched_probe->sent_at.tv_sec) * 1000.0; // convert s to ms
			rtt_ms += (received_at.tv_nsec - matched_probe->sent_at.tv_nsec) / 1000000.0; // convert nanoseconds to ms, add the diff

			printf("REPLY seq=%u time=%.3f ms\n",
				(unsigned int)matched_probe->sequence, rtt_ms);
		}
	}


	close(sock_fd);
	return (0);
}

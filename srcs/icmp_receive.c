#include "icmp.h"

static t_IcmpProbe *find_probe(t_IcmpProbe *probes, uint16_t target, size_t used_count) // finds a saved probe by its sequence number
{
	for(size_t i = 0; i < used_count; ++i)
	{
		if(target == probes[i].sequence)
		{
			return(&probes[i]); // return the matching probe
		}
	}

	return(NULL); // no matching probe was found
}

// returns 1 when readable, 0 on timeout, or -1 on error
static int	icmp_wait_for_packet(int sock_fd, const t_timespec *deadline)
{
	t_timespec		now;
	double			remaining_ms;
	int				timeout_ms;
	int				ret;
	struct pollfd	poll_fd;

	if(clock_gettime(CLOCK_MONOTONIC, &now) == -1) // get time to calculate waiting time
	{
		perror("ft_ping: clock_gettime");
		return(-1);
	}

	remaining_ms = (deadline->tv_sec - now.tv_sec) * 1000.0; // seconds left, converted to ms
	remaining_ms += (deadline->tv_nsec - now.tv_nsec) / 1000000.0; // add the nanosecond difference in ms
	if (remaining_ms <= 0) // deadline reached, stop waiting
		return (0);

	timeout_ms = (int)remaining_ms; // keep the whole milliseconds
	if (timeout_ms < remaining_ms) // round up if there was a fractional part
		timeout_ms++;

	poll_fd.fd = sock_fd; // we poll() our socketfd
	poll_fd.events = POLLIN; // "tell me when there's data to read"

	ret = poll(&poll_fd, 1, timeout_ms);
	if(ret == -1)
	{
		printf("ft_ping: poll: %s\n", strerror(errno));
		return (-1);
	}
	if(ret == 0) // timeout, no packet arrived
		return (0);
	if ((poll_fd.revents & POLLIN) == 0) // socket woke up without readable data
	{
		printf("ft_ping: poll: socket is not readable\n");
		return (-1);
	}
	return (1);
}

ssize_t	icmp_receive(int sock_fd, const struct icmphdr *icmp_header, const t_addrinfo *resolved_addr, const t_timespec *deadline, t_IcmpProbe *probes, size_t used_count, t_IcmpProbe **matched_probe) // receives packet
{
	unsigned char				buffer[2048];
	struct sockaddr_in 			sender;
	socklen_t 					sender_len;
	ssize_t						received;
	int							ret;
	const struct sockaddr_in 	*destination;
	uint16_t 					reply_sequence;
	t_IcmpProbe					*probe;

	memset(&sender, 0, sizeof(sender));
	destination = (const struct sockaddr_in *)resolved_addr->ai_addr;


	while(1)		// awaiting packet loop
	{
		ret = icmp_wait_for_packet(sock_fd, deadline);
		if (ret <= 0)
			return (ret);

		sender_len = sizeof(sender);
		received = recvfrom(sock_fd, buffer, sizeof(buffer), 0,(struct sockaddr *)&sender, &sender_len);
		if (received == -1)
		{
			perror("ft_ping: recv");
			return (-1);
		}

		ret = icmp_inspect_packet(buffer, (size_t)received, icmp_header, &reply_sequence);
		if (ret == 0) // this packet is an answer to my ping
		{
			if(sender.sin_addr.s_addr == destination->sin_addr.s_addr)	// checks if packet is from the IP we pinged
			{
				probe = find_probe(probes, reply_sequence, used_count); // finds the saved probe with the same sequence as the reply
				if (probe == NULL)
					continue ;

				if (probe->replied == 0)
				{
					probe->replied = 1;
				}
				else
					continue ;

				*matched_probe = probe;  // make it possible for icmp_exec() to know which saved probe received the reply
				return(received);
			}
		}
		else					// this packet is something else, so reloop
		{
			continue ;
		}
	}
}

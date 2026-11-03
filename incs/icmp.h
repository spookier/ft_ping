#ifndef ICMP_H
# define ICMP_H

# include "ft_ping.h"

# include <unistd.h>
# include <stdio.h>
# include <string.h>
# include <time.h>
# include <poll.h>
# include <errno.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
#include <signal.h>

extern volatile sig_atomic_t stop_requested;

typedef struct timespec	t_timespec;

typedef struct IcmpProbe
{
	uint16_t sequence;
	t_timespec sent_at;
	int replied;

} t_IcmpProbe;

int	icmp_exec(t_addrinfo *resolved_addr);
void icmp_create_header(struct icmphdr *icmp_header, int sequence_nb);
int	icmp_inspect_packet(const unsigned char *buffer, size_t received,
		const struct icmphdr *icmp_header, uint16_t *reply_sequence);
ssize_t	icmp_receive(int sock_fd, const struct icmphdr *icmp_header,
		const t_addrinfo *resolved_addr, const t_timespec *deadline,
		t_IcmpProbe *probes, size_t used_count, t_IcmpProbe **matched_probe);

#endif

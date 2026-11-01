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

typedef struct timespec	t_timespec;

int	icmp_run(t_addrinfo *resolved_addr);
void icmp_create_header(struct icmphdr *icmp_header);
int	icmp_inspect_packet(const unsigned char *buffer, size_t received, const struct icmphdr *icmp_header);
ssize_t	icmp_receive(int sock_fd, const struct icmphdr *icmp_header, const t_addrinfo *resolved_addr, const t_timespec *sent_at);

#endif

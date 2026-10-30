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

int	exec_icmp(t_addrinfo *resolved_addr);

#endif

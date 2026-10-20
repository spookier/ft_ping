#ifndef ICMP_H
# define ICMP_H

# include "ft_ping.h"

# include <unistd.h>
# include <stdio.h>
# include <string.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>

int	exec_icmp(t_addrinfo *resolved_addr);

#endif

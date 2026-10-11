#ifndef FT_PING_H

# define FT_PING_H

#include <unistd.h>         // close, getpid
#include <stdio.h>
#include <string.h>

#include <sys/types.h>      // basic system types
#include <sys/socket.h>     // socket, sendto, recvfrom
#include <sys/time.h>       // gettimeofday

#include <netdb.h>          // getaddrinfo, freeaddrinfo, gai_strerror
#include <arpa/inet.h>      // inet_ntop, htons, ntohs
#include <netinet/in.h>     // sockaddr_in, IPPROTO_ICMP
#include <netinet/ip.h>     // struct ip
#include <netinet/ip_icmp.h>// struct icmphdr / ICMP defs on Linux

typedef struct addrinfo	t_addrinfo;


int	resolve_hostname(const char *hostname, t_addrinfo **res);

int	cleanup(t_addrinfo **res);

int	open_icmp_socket(void);



#endif

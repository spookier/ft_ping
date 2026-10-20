#ifndef FT_PING_H

# define FT_PING_H

#include <stdio.h>
#include <string.h>

#include <sys/types.h>      // basic system types
#include <sys/socket.h>     // socket, sendto, recvfrom

#include <netdb.h>          // getaddrinfo, freeaddrinfo, gai_strerror
#include <arpa/inet.h>      // inet_ntop, htons, ntohs
#include <netinet/in.h>     // sockaddr_in

typedef struct addrinfo	t_addrinfo;


int	resolve_hostname(const char *hostname, t_addrinfo **resolved_addr);

int	cleanup(t_addrinfo **resolved_addr);



#endif

#ifndef ICMP_H
# define ICMP_H

# include "ft_ping.h"

# include <inttypes.h>
# include <sys/types.h>
# include <time.h>
# include <netinet/ip_icmp.h>
#include <signal.h>

# define ICMP_HEADER_LEN 8
# define PAYLOAD_LEN (64 - ICMP_HEADER_LEN) // 64 byte ICMP packet - 8 byte header = 56

extern volatile sig_atomic_t stop_requested;

typedef struct timespec	t_timespec;

typedef struct IcmpProbe
{
	uint16_t sequence;
	t_timespec sent_at;
	int replied;

} t_IcmpProbe;

typedef struct s_icmp_packet
{
	struct icmphdr	header;
	uint8_t			payload[PAYLOAD_LEN];
}	t_icmp_packet;


int	icmp_exec(t_addrinfo *resolved_addr);
void icmp_create_packet(t_icmp_packet *packet, uint16_t sequence_nb);
int	icmp_inspect_packet(const unsigned char *buffer, size_t received,
		const t_icmp_packet *icmp_packet, uint16_t *reply_sequence);

ssize_t	icmp_receive(int sock_fd, const t_icmp_packet *icmp_header, const t_addrinfo *resolved_addr, const t_timespec *deadline, t_IcmpProbe *probes, size_t used_count, t_IcmpProbe **matched_probe);

#endif

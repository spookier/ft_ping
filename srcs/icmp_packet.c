#include "icmp.h"

#include <string.h>
#include <unistd.h>

static uint16_t icmp_checksum(const void *data, size_t len) // calculates a value used to check if the packet was damaged
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

void icmp_create_packet(t_icmp_packet *packet, uint16_t sequence_nb) // sets up the ICMP packet
{
	uint16_t identifier;

	memset(packet, 0, sizeof(*packet));

	packet->header.type = ICMP_ECHO;

	identifier = getpid() & 0xFFFF;
	packet->header.un.echo.id = htons(identifier);
	packet->header.un.echo.sequence = htons(sequence_nb);

	memcpy(packet->payload, "HELLO", strlen("HELLO"));		// adds HELLO as payload

	packet->header.checksum = htons(icmp_checksum(packet, sizeof(*packet)));

}

int	icmp_inspect_packet(const unsigned char *buffer, size_t received, const t_icmp_packet *icmp_packet, uint16_t *reply_sequence) // inspects the received packet
{
	size_t			ipv4len;
	struct icmphdr 	icmp_reply;
	size_t 			payload_len;

	if (received < 20 || (buffer[0] >> 4) != 4)	// checks if first half-byte is IPv4
		return (-1);

	// [IP HEADER][ICMP HEADER]
	ipv4len = (buffer[0] & 0x0F) * 4; // we get the last half-byte, * 4 to know end of IPv4 packet

	if (ipv4len < 20) // an IPv4 header must be at least 20 bytes
		return (-1);

	if (received < ipv4len + 8) // check if we got the IP header + 8 bytes for ICMP header
		return (-1);

	payload_len = received - ipv4len - sizeof(struct icmphdr);		// calculate received payload size


	if(buffer[ipv4len] == ICMP_ECHOREPLY)
	{
		memcpy(&icmp_reply, &buffer[ipv4len], sizeof(icmp_reply)); // we transform buffer into a struct for ez access

		if(icmp_reply.code == 0 && icmp_reply.un.echo.id == icmp_packet->header.un.echo.id) // check if reply is same as request
		{

			if(payload_len != sizeof(icmp_packet->payload))		// check if sent payload same size as received
				return(2);

			if(memcmp(&buffer[ipv4len + sizeof(struct icmphdr)], icmp_packet->payload, sizeof(icmp_packet->payload)) != 0)	// check if sent payload same content as received
				return(2);

			if(icmp_checksum(&buffer[ipv4len], received - ipv4len) == 0) // checks for corruption, if ICMP is intact
			{
				*reply_sequence = ntohs(icmp_reply.un.echo.sequence);		// save our received seq to compare it with storage
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

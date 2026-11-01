#include "icmp.h"

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

void icmp_create_header(struct icmphdr *icmp_header) // sets up the ICMP header
{
	uint16_t identifier;

	memset(icmp_header, 0, sizeof(*icmp_header));

	icmp_header->type = ICMP_ECHO;

	identifier = getpid() & 0xFFFF;
	icmp_header->un.echo.id = htons(identifier);
	icmp_header->un.echo.sequence = htons(1);
	icmp_header->checksum = htons(icmp_checksum(icmp_header, sizeof(*icmp_header)));
}

int	icmp_inspect_packet(const unsigned char *buffer, size_t received, const struct icmphdr *icmp_header) // inspects the received packet
{
	size_t			ipv4_len;
	struct icmphdr 	icmp_reply;

	if (received < 20 || (buffer[0] >> 4) != 4)	// checks if first half-byte is IPv4
		return (-1);

	// [IP HEADER][ICMP HEADER]
	ipv4_len = (buffer[0] & 0x0F) * 4; // we get the last half-byte, * 4 to know end of IPv4 packet

	if (ipv4_len < 20) // an IPv4 header must be at least 20 bytes
		return (-1);

	if (received < ipv4_len + 8) // check if we got the IP header + 8 bytes for ICMP header
		return (-1);

	if(buffer[ipv4_len] == ICMP_ECHOREPLY)
	{
		memcpy(&icmp_reply, &buffer[ipv4_len], sizeof(icmp_reply)); // we transform buffer into a struct for ez access

		if(icmp_reply.code == 0 && icmp_reply.un.echo.id == icmp_header->un.echo.id && icmp_reply.un.echo.sequence == icmp_header->un.echo.sequence) // check if reply is same as request
		{
			if(icmp_checksum(&buffer[ipv4_len], received - ipv4_len) == 0) // checks for corruption, if ICMP is intact
			{

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

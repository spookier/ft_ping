# ft_ping

An IPv4 implementation of `ping` written from scratch in C

Sends **ICMP Echo Requests** through raw sockets and measures RTT

It uses **GNU inetutils 2.0** as the behavioral reference


## Highlights

- Parses IPv4 and ICMP headers
- Handles network byte order, and filters packets belonging to other processes
- Matches delayed replies by sequence number, rejects duplicates, and measures RTT per request
- Uses `poll()` and `CLOCK_MONOTONIC` deadlines to receive replies without delaying scheduled requests

## Build & Run

```bash
make
sudo ./ft_ping 1.1.1.1
sudo ./ft_ping example.com
sudo ./ft_ping -v example.com
./ft_ping '-?'
```

> Run with `sudo` to allow raw socket creation


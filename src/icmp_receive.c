// Adam Kufel 292345
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "icmp_receive.h"

//This function gets single packet from socket
struct recv_result icmp_receive(int sockfd, u_int16_t ttl) 
{
	rs.is_packet = 0;
	rs.is_not_last_in_seq = 1;
	rs.is_echoreply_type = 0;

	struct sockaddr_in 	sender;	
	socklen_t sender_len = sizeof(sender);
	u_int8_t buffer[IP_MAXPACKET];

	char sender_ip_str[20]; 

	ssize_t packet_len;
	packet_len = recvfrom(sockfd, 
						  buffer, //packet as a bytes seq
						  IP_MAXPACKET, 
						  MSG_DONTWAIT, 
						  (struct sockaddr*)&sender, //information about sender
						  &sender_len);

	if (packet_len > 0) {
		struct ip* ip_header = (struct ip*) buffer;
		u_int8_t* icmp_packet = buffer + 4 * ip_header->ip_hl;
		struct icmphdr* icmp_header = (struct icmphdr*)(icmp_packet);

		//indicates received packet 
		if (icmp_header->type == ICMP_TIME_EXCEEDED || icmp_header->type == ICMP_ECHOREPLY) 
		{
			if (icmp_header->type == ICMP_TIME_EXCEEDED) 
			{
				// Go further because of presence of nested ip header in icmp packet of time exceeded type
				icmp_packet += 4 * ip_header->ip_hl + sizeof(icmp_header);
				icmp_header = (struct icmphdr*)(icmp_packet);
			}
			if (icmp_header->un.echo.sequence == ttl && icmp_header->un.echo.id == (uint16_t)getpid()) 
			{
				// convert received ip address in bytes to human-readable string
				inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, sizeof(sender_ip_str));

				rs.is_packet = 1;
				strcpy(rs.recv_ip_address, sender_ip_str);

				if (icmp_header->type == ICMP_ECHOREPLY)
					rs.is_echoreply_type = 1;

				return rs;
			}
		}
	}
	//indicates that socket does not have any more packet at this moment
	else if (packet_len == -1) 
	{
		rs.is_not_last_in_seq = 0;
		return rs;
	}
	else 
	{
		fprintf(stderr, "recv error: %s\n", strerror(errno)); 
		exit(EXIT_FAILURE);
	}
	return rs;
}

// Adam Kufel 292345
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "icmp_checksum.h"

void icmp_send(int seq_num, int sockfd, char *ip_address) 
{
    //create icmp header/packet
    struct icmphdr icmp_header;
    icmp_header.type = ICMP_ECHO;
    icmp_header.code = 0;
    icmp_header.un.echo.sequence = (uint16_t)(seq_num);
    icmp_header.un.echo.id = (uint16_t)getpid();
    icmp_header.checksum = 0;
    icmp_header.checksum = compute_icmp_checksum((u_int16_t*)&icmp_header, sizeof(icmp_header));

    //prepare recipient data as sockaddr_in struct
    struct sockaddr_in recipient;
    bzero(&recipient, sizeof(recipient));
    recipient.sin_family = AF_INET;
    inet_pton(AF_INET, ip_address, &recipient.sin_addr);

    //send icmp packet
    ssize_t bytes_sent = sendto(sockfd, &icmp_header, sizeof(icmp_header), 0, 
                               (struct sockaddr*)&recipient, sizeof(recipient));
    if (bytes_sent < 0) 
    {
  		fprintf(stderr, "sendto error: %s\n", strerror(errno));
      exit(EXIT_FAILURE); 
    }
}
// Adam Kufel 292345
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include "icmp_receive.h"
#include "icmp_send.h"

#define START_TTL 1
#define MAX_TTL 30

#define MAX_RECV_TIME 1

#define MAX_PACKETS_IN_SEQ 3
#define IP_MAX_LENGTH 20


int trace_found = 0;

void print_traceroute_result(int ttl, u_int8_t recv_packet_count, 
                             char recv_ip_addresses[MAX_PACKETS_IN_SEQ][IP_MAX_LENGTH], 
                             int summary_time);

void traceroute(char* ip_address) 
{
    //create raw socket
	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0) 
    {
		fprintf(stderr, "socket error: %s\n", strerror(errno)); 
        exit(EXIT_FAILURE);
    }
 
    //begin sending packets with ttls 
    for (int ttl = START_TTL; ttl <= MAX_TTL; ttl++)
    {
        int summary_time;
        int is_last_in_seq = 1;
        u_int8_t recv_packet_count;
        char recv_ip_addresses[MAX_PACKETS_IN_SEQ][IP_MAX_LENGTH] = {".", ".", "."};
        struct recv_result recv_packet;

        //set file descriptors for listening on created socket
        fd_set descriptors;
        FD_ZERO (&descriptors);
        FD_SET (sockfd, &descriptors);

        //set max timeval 
        struct timeval tv; tv.tv_sec = MAX_RECV_TIME; tv.tv_usec = 0;

        // set ttl for packets in this seq
        setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));

        //send 3 icmp packets - keep origin ttl value in header as sequence number
        for (u_int16_t i = 0; i < 3; i++)
            icmp_send(ttl, sockfd, ip_address);

        summary_time = 0;
        recv_packet_count = 0;

        //receive packets for 1s timeval
        while(is_last_in_seq && recv_packet_count < 3) 
        {
            int ready = select(sockfd+1, &descriptors, NULL, NULL, &tv);
            if (ready < 0)
            {
                fprintf(stderr, "select error");
                exit(EXIT_FAILURE);
            }
            //timeout
            else if (ready == 0) 
            {
                is_last_in_seq = 0;
            }
            else
            {        
                recv_packet = icmp_receive(sockfd, ttl);
                is_last_in_seq = recv_packet.is_not_last_in_seq;
                if (recv_packet.is_packet)
                {
                    strcpy(recv_ip_addresses[recv_packet_count], recv_packet.recv_ip_address);
                    recv_packet_count++;
                    summary_time += (1000 - tv.tv_usec / 1000);
                    if (recv_packet.is_echoreply_type)
                    {
                        trace_found = 1;
                    }
                }
            }
        }
        print_traceroute_result(ttl, recv_packet_count, recv_ip_addresses, summary_time);

        if (trace_found)
            break;
    }
}

void print_traceroute_result(int ttl, u_int8_t recv_packet_count, 
                             char recv_ip_addresses[MAX_PACKETS_IN_SEQ][IP_MAX_LENGTH], 
                             int summary_time)
{
    printf("%d. ", ttl);
    //print ip addr of routers of recv packets
    if (recv_packet_count == 0) 
    {
        printf("*\n");
    }
    else 
    {    
        printf("%s ", recv_ip_addresses[0]);
        if(strcmp(recv_ip_addresses[0], recv_ip_addresses[1]) != 0)
        {
            if(strcmp(recv_ip_addresses[1], ".") != 0)
                printf("%s ", recv_ip_addresses[1]);
        }
        if(strcmp(recv_ip_addresses[0], recv_ip_addresses[2]) != 0 || 
           strcmp(recv_ip_addresses[1], recv_ip_addresses[2]) != 0)
        {
            if(strcmp(recv_ip_addresses[2], ".") != 0)
                printf("%s ", recv_ip_addresses[2]);
        }    
        //print avg time in all 3 sent packets received
        if (recv_packet_count == MAX_PACKETS_IN_SEQ)
        {
            printf("%d ms\n", summary_time / MAX_PACKETS_IN_SEQ);
        }
        else 
        {
            printf("???\n");
        }
    }
}

int check_ip_address_validity(char* ip_address) 
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip_address, &(sa.sin_addr));
    return result != 0;
}

int main(int argc, char **argv)
{      
    //validate number of arguments  
    if (argc != 2) {
		printf("argument error: expected: 2 <> actual: %d\n", argc); 
		return EXIT_FAILURE;
    }
    if (!check_ip_address_validity(argv[1]))
    {
        printf("argument error: %s is not valid ip address\n", argv[1]);
        return EXIT_FAILURE;
    }
    traceroute(argv[1]);
    
    return EXIT_SUCCESS;
}
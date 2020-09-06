#pragma once
#include "recv_result.h"

struct recv_result icmp_receive(int sockfd, u_int16_t ttl);
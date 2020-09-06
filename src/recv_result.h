#pragma once
#include <sys/types.h>

struct recv_result {
	u_int8_t is_packet;
	char recv_ip_address[20];
	u_int8_t is_not_last_in_seq;
	u_int8_t is_echoreply_type;
} rs;

#ifndef PACKET_DECODE_H
#define PACKET_DECODE_H

#include "decode_ipv4.h"
#include "decode_ipv6.h"
#include "decode_vlan.h"
#include "decode_ethernet.h"

typedef struct packet_t {
} packet_t;

void packet_decode(packet_t *pkt, const char *payload, uint16_t len);

#endif


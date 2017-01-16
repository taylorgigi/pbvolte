#ifndef PACKET_DECODE_H
#define PACKET_DECODE_H

#include "decode.h"
#include "decode_ethernet.h"
#include "decode_ipv4.h"
#include "decode_ipv6.h"
#include "decode_vlan.h"
#include "decode_mpls.h"
#include <stdint.h>

typedef struct packet_t {
	uint8_t proto;
	ethernet_hdr *ethh;
	ipv4_hdr     *ip4h;
	ipv6_hdr     *ip6h;
	vlan_hdr     *vlanh;
	address_t    src;
	address_t    dst;
} packet_t;

int decode_ethernet(packet_t *pkt, uint8_t *payload, uint16_t len);
int decode_ipv4(packet_t *pkt, uint8_t *payload, uint16_t len);
int decode_ipv6(packet_t *pkt, uint8_t *payload, uint16_t len);
int decode_vlan(packet_t *pkt, uint8_t *payload, uint16_t len);
int decode_mpls(packet_t *pkt, uint8_t *payload, uint16_t len);
int packet_decode(packet_t *pkt, uint8_t *payload, uint16_t len);

#endif


#include "packet_decode.h"
#include "decode_ethernet.h"
#include <stdio.h>
#include <stdlib.h>

int decode_ethernet(packet_t *pkt, uint8_t *payload, uint16_t len) {
	if(len < ETHERNET_HEADER_LEN) {
		return -1;
	}
	pkt->ethh = (ethernet_hdr *)payload;
	if(pkt->ethh == NULL) {
		return -1;
	}
	uint16_t type = ntohs(pkt->ethh->eth_type);
	switch(type) {
		case ETHERNET_TYPE_IP:
			return decode_ipv4(pkt, payload + ETHERNET_HEADER_LEN, len - ETHERNET_HEADER_LEN);
		case ETHERNET_TYPE_IPV6:
			return decode_ipv6(pkt, payload + ETHERNET_HEADER_LEN, len - ETHERNET_HEADER_LEN);
		case ETHERNET_TYPE_VLAN:
		case ETHERNET_TYPE_8021QINQ:
			return decode_vlan(pkt, payload + ETHERNET_HEADER_LEN, len - ETHERNET_HEADER_LEN);
		case ETHERNET_TYPE_MPLS_UNICAST:
		case ETHERNET_TYPE_MPLS_MULTICAST:
			return decode_mpls(pkt, payload + ETHERNET_HEADER_LEN, len - ETHERNET_HEADER_LEN);
		default:
			printf("ether type 0x%04x not supported,%s,%d\n",type,__FILE__,__LINE__);
			return -1;
	}
	return 0;
}


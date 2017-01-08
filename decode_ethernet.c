#include "packet_decode.h"
#include "decode_ethernet.h"
#include <stdio.h>
#include <stdlib.h>

int decode_ethernet(packet_t *pkt, uint8_t *payload, uint16_t len) {
	if(len < ETHERNET_HEADER_LEN) {
		return -1;
	}
	pkt->ethhdr = (ethernet_hdr *)payload;
	if(pkt->ethhdr == NULL) {
		return -1;
	}
	uint16_t type = ntohs(pkt->ethhdr->eth_type);
	switch(type) {
		case ETHERNET_TYPE_IP:
			decode_ipv4(pkt, payload + ETHERNET_HEADER_LEN, len - ETHERNET_HEADER_LEN);
			break;
		case ETHERNET_TYPE_IPV6:
			decode_ipv6(pkt, payload + ETHERNET_HEADER_LEN, len - ETHERNET_HEADER_LEN);
			break;
		case ETHERNET_TYPE_VLAN:
		case ETHERNET_TYPE_8021QINQ:
			decode_vlan(pkt, payload + ETHERNET_HEADER_LEN, len - ETHERNET_HEADER_LEN);
			break;
		case ETHERNET_TYPE_MPLS_UNICAST:
		case ETHERNET_TYPE_MPLS_MULTICAST:
			decode_mpls(pkt, payload + ETHERNET_HEADER_LEN, len - ETHERNET_HEADER_LEN);
			break;
		default:
			printf("ether type 0x%04x not supported,%s,%d\n",type,__FILE__,__LINE__);
	}
	return 0;
}


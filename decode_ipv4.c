#include "decode_ipv4.h"
#include "packet_decode.h"
#include "decode.h"
#include <stdio.h>

int decode_ipv4(packet_t *pkt, uint8_t *payload, uint16_t len) {
	if(len < IPV4_HEADER_LEN) { // ipv4 packet too small
		printf("decode_ipv4 error, packet too small of %u bytes, %s, %d\n", len, __FILE__, __LINE__);
		return -1;
	}
	if(IP_GET_RAW_VER(payload) != 4) {
		printf("decode_ipv4 error, wrong ip version, %s, %d\n", __FILE__, __LINE__);
		return -1;
	}
	pkt->ip4h = (ipv4_hdr *)payload;
	if(IPV4_GET_HLEN(pkt) < IPV4_HEADER_LEN) {
		printf("decode_ipv4 error, wrong ip header length, %s, %d\n", __FILE__, __LINE__);
		return -1;
	}
	if(IPV4_GET_IPLEN(pkt) < IPV4_GET_HLEN(pkt)) {
		printf("decode_ipv4 error, ip total length smaller than header length, %s, %d\n", __FILE__, __LINE__);
		return -1;
	}
	if(len < IPV4_GET_IPLEN(pkt)) {
		printf("decode_ipv4 error, packet truncated, %s, %d\n", __FILE__, __LINE__);
		return -1;
	}
	SET_IPV4_SRC_ADDR(pkt,&pkt->src);
	SET_IPV4_DST_ADDR(pkt,&pkt->dst);
	// to-do: decode ipv4 options, ignored
	pkt->proto = IPV4_GET_IPPROTO(pkt);
	// if a fragment, pass off for re-assemply
	if(IPV4_GET_IPOFFSET(pkt) > 0 || IPV4_GET_MF(pkt) == 1) {
	}
	// check what next decoder to invoke
	switch(IPV4_GET_IPPROTO(pkt)) {
		case IPPROTO_TCP:
		case IPPROTO_UDP:
		case IPPROTO_ICMP:
		case IPPROTO_GRE:
		case IPPROTO_SCTP:
		case IPPROTO_IPV6:
		case IPPROTO_IP:
		case IPPROTO_ICMPV6:
		default:
			break;
	}

	return 0;
}


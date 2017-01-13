#include "packet_decode.h"
#include <stdio.h>

// the entrance of decoder
int packet_decode(packet_t *pkt, uint8_t *payload, uint16_t len) {
	int code;
	// ip4,ip6,vlan,mpls ite, was decoded in decode_ethernet
	code = decode_ethernet(pkt, payload, len);
	if(code < 0) { // decode failed
		printf("decode_ethernet error, %s, %d\n", __FILE__, __LINE__);
		return -1;
	}
	return 0;
}


#include "packet_decode.h"
#include <stdio.h>

int packet_decode(packet_t *pkt, uint8_t *payload, uint16_t len) {
	int code;
	code = decode_ethernet(pkt, payload, len);
	if(code < 0) { // decode failed
		printf("decode_ethernet error, %s, %d\n", __FILE__, __LINE__);
		return -1;
	}
}


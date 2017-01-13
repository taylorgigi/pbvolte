#ifndef DECODE_INCLUDE_H
#define DECODE_INCLUDE_H

#include <stdint.h>

/* Address */
typedef struct address_s {
	char family;
	union {
		uint32_t       address_un_data32[4]; /* type-specific field */
		uint16_t       address_un_data16[8]; /* type-specific field */
		uint8_t        address_un_data8[16]; /* type-specific field */
	} address;
} address_t;

#define addr_data32 address.address_un_data32
#define addr_data16 address.address_un_data16
#define addr_data8  address.address_un_data8

/* Set the IPv4 addresses into the Addrs of the Packet.
 * Make sure p->ip4h is initialized and validated.
 *
 * We set the rest of the struct to 0 so we can
 * prevent using memset. */
#define SET_IPV4_SRC_ADDR(p, a) do {                              \
	(a)->family = AF_INET;                                    \
	(a)->addr_data32[0] = (uint32_t)(p)->ip4h->s_ip_src.s_addr; \
	(a)->addr_data32[1] = 0;                                  \
	(a)->addr_data32[2] = 0;                                  \
	(a)->addr_data32[3] = 0;                                  \
} while (0)

#define SET_IPV4_DST_ADDR(p, a) do {                              \
	(a)->family = AF_INET;                                    \
	(a)->addr_data32[0] = (uint32_t)(p)->ip4h->s_ip_dst.s_addr; \
	(a)->addr_data32[1] = 0;                                  \
	(a)->addr_data32[2] = 0;                                  \
	(a)->addr_data32[3] = 0;                                  \
} while (0)

/*Given a packet pkt offset to the start of the ip header in a packet
 *  *We determine the ip version. */
#define IP_GET_RAW_VER(pkt) ((((pkt)[0] & 0xf0) >> 4))

#endif


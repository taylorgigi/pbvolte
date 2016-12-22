#ifndef DECODE_VLAN_H
#define DECODE_VLAN_H

#include <stdint.h>

/** VLAN header length */
#define VLAN_HEADER_LEN    4

/** Vlan macros to access Vlan priority, Vlan CFI and VID */
#define GET_VLAN_PRIORITY(vlanh)    ((ntohs((vlanh)->vlan_cfi) & 0xe000) >> 13)
#define GET_VLAN_CFI(vlanh)         ((ntohs((vlanh)->vlan_cfi) & 0x0100) >> 12)
#define GET_VLAN_ID(vlanh)          ((uint16_t)(ntohs((vlanh)->vlan_cfi) & 0x0FFF))
#define GET_VLAN_PROTO(vlanh)       ((ntohs((vlanh)->protocol)))

typedef struct vlan_hdr {
	uint16_t vlan_cfi;
	uint16_t protocol; /**< protocol field */
} __attribute__((__packed__)) vlan_hdr;

#endif


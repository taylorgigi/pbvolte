#ifndef DECODE_ETHERNET_H
#define DECODE_ETHERNET_H

#include "packet_decode.h"
#include <stdint.h>

#define ETHERNET_HEADER_LEN           14

/* Cisco Fabric Path / DCE header length. */
#define ETHERNET_DCE_HEADER_LEN       ETHERNET_HEADER_LEN + 2

/* Ethernet types -- taken from Snort and Libdnet */
#define ETHERNET_TYPE_PUP             0x0200 /* PUP protocol */
#define ETHERNET_TYPE_IP              0x0800
#define ETHERNET_TYPE_ARP             0x0806
#define ETHERNET_TYPE_BRIDGE          0x6558 /* transparant ethernet bridge (GRE) */
#define ETHERNET_TYPE_REVARP          0x8035
#define ETHERNET_TYPE_EAPOL           0x888e
#define ETHERNET_TYPE_IPV6            0x86dd
#define ETHERNET_TYPE_IPX             0x8137
#define ETHERNET_TYPE_PPPOE_DISC      0x8863 /* discovery stage */
#define ETHERNET_TYPE_PPPOE_SESS      0x8864 /* session stage */
#define ETHERNET_TYPE_8021AD          0x88a8
#define ETHERNET_TYPE_8021Q           0x8100
#define ETHERNET_TYPE_LOOP            0x9000
#define ETHERNET_TYPE_8021QINQ        0x9100
#define ETHERNET_TYPE_ERSPAN          0x88BE
#define ETHERNET_TYPE_DCE             0x8903 /* Data center ethernet,
                                              * Cisco Fabric Path */
/** Vlan type */
#define ETHERNET_TYPE_VLAN            0x8100
#define ETHERNET_TYPE_MPLS_UNICAST    0x8847
#define ETHERNET_TYPE_MPLS_MULTICAST  0x8848


typedef struct ethernet_hdr {
	uint8_t eth_dst[6];
	uint8_t eth_src[6];
	uint16_t eth_type;
} __attribute__((__packed__)) ethernet_hdr;

int decode_ethernet(packet_t *pkt, uint8_t *payload, uint16_t len);

#endif


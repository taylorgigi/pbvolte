#ifndef DECODE_IPV6_H
#define DECODE_IPV6_H

#include <stdint.h>
#include <netinet/in.h>

#define IPV6_HEADER_LEN            40
#define	IPV6_MAXPACKET	           65535 /* maximum packet size */
#define IPV6_MAX_OPT               40

typedef struct ipv6_hdr {
	union {
		struct ip6_un1_ {
			uint32_t ip6_un1_flow; /* 20 bits of flow-ID */
			uint16_t ip6_un1_plen; /* payload length */
			uint8_t  ip6_un1_nxt;  /* next header */
			uint8_t  ip6_un1_hlim; /* hop limit */
		} ip6_un1;
		uint8_t ip6_un2_vfc;   /* 4 bits version, top 4 bits class */
	} ip6_hdrun;

	union {
		struct {
			uint32_t ip6_src[4];
			uint32_t ip6_dst[4];
		} ip6_un2;
		uint16_t ip6_addrs[16];
	} ip6_hdrun2;
} __attribute__((__packed__)) ipv6_hdr;

#define s_ip6_src                       ip6_hdrun2.ip6_un2.ip6_src
#define s_ip6_dst                       ip6_hdrun2.ip6_un2.ip6_dst
#define s_ip6_addrs                     ip6_hdrun2.ip6_addrs

#define s_ip6_vfc                       ip6_hdrun.ip6_un2_vfc
#define s_ip6_flow                      ip6_hdrun.ip6_un1.ip6_un1_flow
#define s_ip6_plen                      ip6_hdrun.ip6_un1.ip6_un1_plen
#define s_ip6_nxt                       ip6_hdrun.ip6_un1.ip6_un1_nxt
#define s_ip6_hlim                      ip6_hdrun.ip6_un1.ip6_un1_hlim

#define IPV6_GET_RAW_VER(ip6h)          (((ip6h)->s_ip6_vfc & 0xf0) >> 4)
#define IPV6_GET_RAW_CLASS(ip6h)        ((ntohl((ip6h)->s_ip6_flow) & 0x0FF00000) >> 20)
#define IPV6_GET_RAW_FLOW(ip6h)         (ntohl((ip6h)->s_ip6_flow) & 0x000FFFFF)
#define IPV6_GET_RAW_NH(ip6h)           ((ip6h)->s_ip6_nxt)
#define IPV6_GET_RAW_PLEN(ip6h)         (ntohs((ip6h)->s_ip6_plen))
#define IPV6_GET_RAW_HLIM(ip6h)         ((ip6h)->s_ip6_hlim)
#define IPV6_GET_RAW_SRC(ip6h)          ((ip6h)->s_ip6_src)
#define IPV6_GET_RAW_DST(ip6h)          ((ip6h)->s_ip6_dst)

#define IPV6_SET_RAW_VER(ip6h, value)   ((ip6h)->s_ip6_vfc = (((ip6h)->s_ip6_vfc & 0x0f) | (value << 4)))
#define IPV6_SET_RAW_NH(ip6h, value)    ((ip6h)->s_ip6_nxt = (value))

#define IPV6_SET_L4PROTO(p,proto)       (p)->ip6vars.l4proto = proto

/* ONLY call these functions after making sure that:
 * 1. p->ip6h is set
 * 2. p->ip6h is valid (len is correct)
 */
#define IPV6_GET_VER(p) \
	IPV6_GET_RAW_VER((p)->ip6h)
#define IPV6_GET_CLASS(p) \
	IPV6_GET_RAW_CLASS((p)->ip6h)
#define IPV6_GET_FLOW(p) \
	IPV6_GET_RAW_FLOW((p)->ip6h)
#define IPV6_GET_NH(p) \
	(IPV6_GET_RAW_NH((p)->ip6h))
#define IPV6_GET_PLEN(p) \
	IPV6_GET_RAW_PLEN((p)->ip6h)
#define IPV6_GET_HLIM(p) \
	(IPV6_GET_RAW_HLIM((p)->ip6h))
/* XXX */
#define IPV6_GET_L4PROTO(p) \
	((p)->ip6vars.l4proto)

typedef struct ipv6_vars {
	uint8_t ip_opts_len;
	uint8_t l4proto;      /* the proto after the extension headers
			       * store while decoding so we don't have
			       * to loop through the exthdrs all the time */
} ipv6_vars;

#define CLEAR_IPV6_PACKET(p) do { \
    (p)->ip6h = NULL; \
    (p)->ip6vars.ip_opts_len = 0; \
    (p)->ip6vars.l4proto = 0; \
    memset(&(p)->ip6eh, 0x00, sizeof((p)->ip6eh)); \
} while (0)

/* Fragment header */
typedef struct ipv6_frag_hdr {
    uint8_t  ip6fh_nxt;             /* next header */
    uint8_t  ip6fh_reserved;        /* reserved field */
    uint16_t ip6fh_offlg;           /* offset, reserved, and flag */
    uint32_t ip6fh_ident;           /* identification */
} __attribute__((__packed__)) ipv6_frag_hdr;

#define IPV6_EXTHDR_GET_FH_NH(p)            (p)->ip6eh.fh_nh
#define IPV6_EXTHDR_GET_FH_OFFSET(p)        (p)->ip6eh.fh_offset
#define IPV6_EXTHDR_GET_FH_FLAG(p)          (p)->ip6eh.fh_more_frags_set
#define IPV6_EXTHDR_GET_FH_ID(p)            (p)->ip6eh.fh_id

/* rfc 1826 */
typedef struct ipv6_auth_hdr {
	uint8_t ip6ah_nxt;              /* next header */
	uint8_t ip6ah_len;              /* header length in units of 8 bytes, not
					   including first 8 bytes. */
	uint16_t ip6ah_reserved;        /* reserved for future use */
	uint32_t ip6ah_spi;             /* SECURITY PARAMETERS INDEX (SPI) */
	uint32_t ip6ah_seq;             /* sequence number */
} __attribute__((__packed__)) ipv6_auth_hdr;

typedef struct ipv6_esp_hdr {
	uint32_t ip6esph_spi;           /* SECURITY PARAMETERS INDEX (SPI) */
	uint32_t ip6esph_seq;           /* sequence number */
} __attribute__((__packed__)) ipv6_esp_hdr;

typedef struct ipv6_route_hdr {
	uint8_t ip6rh_nxt;               /* next header */
	uint8_t ip6rh_len;               /* header length in units of 8 bytes, not
					    including first 8 bytes. */
	uint8_t ip6rh_type;              /* routing type */
	uint8_t ip6rh_segsleft;          /* segments left */
} __attribute__((__packed__)) ipv6_route_hdr;

/* Hop-by-Hop header and Destination Options header use options that are
 *  * defined here. */
#define IPV6OPT_PAD1                  0x00
#define IPV6OPT_PADN                  0x01
#define IPV6OPT_RA                    0x05
#define IPV6OPT_JUMBO                 0xC2
#define IPV6OPT_HAO                   0xC9

/* Home Address Option */
typedef struct ipv6_opt_hao {
	uint8_t ip6hao_type;             /* Option type */
	uint8_t ip6hao_len;              /* Option Data len (excludes type and len) */
	struct in6_addr ip6hao_hoa;       /* Home address. */
} ipv6_opt_hao;

/* Router Alert Option */
typedef struct ipv6_opt_ra {
	uint8_t ip6ra_type;             /* Option type */
	uint8_t ip6ra_len;              /* Option Data len (excludes type and len) */
	uint16_t ip6ra_value;           /* Router Alert value */
} ipv6_opt_ra;

/* Jumbo Option */
typedef struct ipv6_opt_jumbo {
	uint8_t ip6j_type;             /* Option type */
	uint8_t ip6j_len;              /* Option Data len (excludes type and len) */
	uint32_t ip6j_payload_len;     /* Jumbo Payload Length */
} ipv6_opt_jumbo;

typedef struct ipv6_hop_opts_hdr {
	uint8_t ip6hh_nxt;              /* next header */
	uint8_t ip6hh_len;              /* header length in units of 8 bytes, not
					   including first 8 bytes. */
} __attribute__((__packed__)) ipv6_hop_opts_hdr;

typedef struct ipv6_dst_opts_hdr {
	uint8_t ip6dh_nxt;              /* next header */
	uint8_t ip6dh_len;              /* header length in units of 8 bytes, not
					   including first 8 bytes. */
} __attribute__((__packed__)) ipv6_dst_opts_hdr;

typedef struct ipv6_gen_opt_hdr {
	uint8_t type;
	uint8_t next;
	uint8_t len;
	uint8_t *data;
} __attribute__((__packed__)) ipv6_gen_opt_hdr;

typedef struct ipv6_ext_hdrs {
	_Bool rh_set;
	uint8_t rh_type;

	_Bool fh_set;
	_Bool fh_more_frags_set;
	uint8_t fh_nh;

	uint8_t fh_prev_nh;
	uint16_t fh_prev_hdr_offset;

	uint16_t fh_header_offset;
	uint16_t fh_data_offset;
	uint16_t fh_data_len;

	/* In fh_offset we store the offset of this extension into the packet past
	 * the ipv6 header. We use it in defrag for creating a defragmented packet
	 * without the frag header */
	uint16_t fh_offset;
	uint32_t fh_id;
} ipv6_ext_hdrs;

#define IPV6_EXTHDR_SET_FH(p)       (p)->ip6eh.fh_set = TRUE
#define IPV6_EXTHDR_ISSET_FH(p)     (p)->ip6eh.fh_set
#define IPV6_EXTHDR_SET_RH(p)       (p)->ip6eh.rh_set = TRUE
#define IPV6_EXTHDR_ISSET_RH(p)     (p)->ip6eh.rh_set

#endif


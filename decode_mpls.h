#ifndef DECODE_MPLS_INCLUDE_H
#define DECODE_MPLS_INCLUDE_H

#define MPLS_HEADER_LEN         4
#define MPLS_PW_LEN             4
#define MPLS_MAX_RESERVED_LABEL 15

#define MPLS_LABEL_IPV4         0
#define MPLS_LABEL_ROUTER_ALERT 1
#define MPLS_LABEL_IPV6         2
#define MPLS_LABEL_NULL         3

#define MPLS_LABEL(shim)  ntohl(shim) >> 12
#define MPLS_BOTTOM(shim) ((ntohl(shim) >> 8)& 0x1)

/* Inner protocol guessing values. */
#define MPLS_PROTO_ETHERNET_PW  0
#define MPLS_PROTO_IPV4         4
#define MPLS_PROTO_IPV6         6

#endif


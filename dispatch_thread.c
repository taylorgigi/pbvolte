#include "list.h"
#include "global.h"
#include "pkt_pool.h"
#include "capture_thread.h"
#include "dispatch_thread.h"
#include "decode_ipv4.h"
#include "decode_ipv6.h"
#include "decode_ethernet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

// generate key by src/dst ip
int gen_key_by_ip(node_t *n, dis_key *key) {
	pkt_buffer *pktbf = NULL;
	ethernet_hdr *eth_hdr = NULL;
	ipv4_hdr *ip4hdr = NULL;
	ipv6_hdr *ip6hdr = NULL;
	eth_hdr = (ethernet_hdr*)pktbf->pkt;
	pktbf = (pkt_buffer*)n->data;
	eth_hdr = (ethernet_hdr*)pktbf->pkt;
	uint16_t type = ntohs(eth_hdr->eth_type);
	char *ptr = pktbf->pkt + ETHERNET_HEADER_LEN;
AGAIN:
	switch(type) {
		case ETHERNET_TYPE_IP:
			ip4hdr = (ipv4_hdr*)ptr;
			key->ip_sum = IPV4_GET_RAW_IPSRC_U32(ip4hdr) + IPV4_GET_RAW_IPDST_U32(ip4hdr);
			break;
		case ETHERNET_TYPE_IPV6:
			ip6hdr = (ipv6_hdr*)(ptr);
			key->ip_sum = IPV6_GET_RAW_SRC(ip6hdr)[0] + IPV6_GET_RAW_DST(ip6hdr)[0];
			break;
		case ETHERNET_TYPE_VLAN:
		case ETHERNET_TYPE_8021QINQ:
			break;
		case ETHERNET_TYPE_MPLS_UNICAST:
		case ETHERNET_TYPE_MPLS_MULTICAST:
			break;
		default:
			printf("ether type 0x%04x not supported,%s,%d\n",type,__FILE__,__LINE__);
			return -1;
	}
}

// dispatch packet to transaction thread according to src/dst ip pair
void *dispatch_thread(void *arg) {
	// to-do: set cpu affinity
	pthread_detach(pthread_self());
	rte_atomic32_inc(&prog_ctl.thr_num);
	node_t *n = NULL;
	dis_key key;
	int keysz = sizeof(dis_key);
	// keep running?
	while(rte_atomic32_read(&prog_ctl.run)) {
		while((n = list_pop(&dispatch_queue)) == NULL) {
			;// do nothing
		}
		memset(&key, 0, keysz);
		gen_key_by_ip(n, &key);
	}
	// cleanup and exit
	rte_atomic32_dec(&prog_ctl.thr_num);
	pthread_exit(NULL);
	return NULL;
}


#include "list.h"
#include "hash.h"
#include "config.h"
#include "global.h"
#include "pkt_pool.h"
#include "decode_ipv4.h"
#include "decode_ipv6.h"
#include "decode_vlan.h"
#include "decode_ethernet.h"
#include "capture_thread.h"
#include "transaction_thread.h"
#include "dispatch_thread.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

uint32_t get_transac_queue(node_t *n) {
	int i;
	uint32_t hash_val;
	uint64_t ip4sum;
	pkt_buffer *pktbf = NULL;
	ethernet_hdr *eth_hdr = NULL;
	ipv4_hdr *ip4hdr = NULL;
	ipv6_hdr *ip6hdr = NULL;
	vlan_hdr *vlhdr = NULL;
	eth_hdr = (ethernet_hdr*)pktbf->pkt;
	pktbf = (pkt_buffer*)n->data;
	eth_hdr = (ethernet_hdr*)pktbf->pkt;
	uint16_t type = ntohs(eth_hdr->eth_type);
	const char *ptr = pktbf->pkt + ETHERNET_HEADER_LEN;
	uint32_t ip6sum[4];
FUCK_AGAIN:
	switch(type) {
		case ETHERNET_TYPE_IP:
			ip4hdr = (ipv4_hdr*)ptr;
			break;
		case ETHERNET_TYPE_IPV6:
			ip6hdr = (ipv6_hdr*)(ptr);
			break;
		case ETHERNET_TYPE_VLAN:
		case ETHERNET_TYPE_8021QINQ:
			vlhdr = (vlan_hdr*)ptr;
			ptr += VLAN_HEADER_LEN;
			type = ntohs(vlhdr->protocol);
			goto FUCK_AGAIN;
			break;
		case ETHERNET_TYPE_ARP:
			break;
		case ETHERNET_TYPE_MPLS_UNICAST:
		case ETHERNET_TYPE_MPLS_MULTICAST:
			break;
		default:
			printf("ether type 0x%04x not supported,%s,%d\n",type,__FILE__,__LINE__);
			break;
	}
	// balance by ip pair
	if(ip4hdr != NULL) {
		ip4sum = IPV4_GET_RAW_IPSRC_U32(ip4hdr) + IPV4_GET_RAW_IPDST_U32(ip4hdr);
		hash_val = hash(&ip4sum, 8);
		return (hash_val % glb_config.nb_thr);
	}
	else if(ip6hdr != NULL) {
		for(i=0; i<4; ++i) {
			ip6sum[i] = (IPV6_GET_RAW_SRC(ip6hdr)[i] + IPV6_GET_RAW_DST(ip6hdr)[i]);
		}
		hash_val = hash(&ip6sum, 16);
		return (hash_val % glb_config.nb_thr);
	}
	// if neither ipv4 nor ipv6 packet, give packet to transaction thread0.
	return 0;
}

// dispatch packet to transaction thread according to src/dst ip pair
void *dispatch_thread(void *arg) {
	// to-do: set cpu affinity
	uint32_t pos;
	list_t *q = NULL;
	node_t *n = NULL;
	pthread_detach(pthread_self());
	rte_atomic32_inc(&prog_ctl.thr_num);
	struct timespec req = {0, 10};
	struct timeval old, cur;
	int delay;

	gettimeofday(&old, NULL);
	// keep running?
	while(rte_atomic32_read(&prog_ctl.run)) {
		while((n = list_pop(&dispatch_queue)) == NULL) {
			// to-do: there may be a performance problem with nanosleep
			nanosleep(&req, NULL);
		}
		// pkt_pool usage statistic ====>
		gettimeofday(&cur, NULL);
		delay = cur.tv_sec - old.tv_sec;
		if(delay >= 60) {
			printf("[DEBUG] pkt pool usage rate %.2f%% %d/%d (used/total)", (rte_atomic32_read(&pack_pool.used)/(float)rte_atomic32_read(&pack_pool.size))*100,rte_atomic32_read(&pack_pool.used), rte_atomic32_read(&pack_pool.size));
			old = cur;
		}
		// <==== pkt_pool usage statistic
		// balancly putting to transaction queue ====>
		pos = get_transac_queue(n);
printf("[DEBUG] %u\n", pos);
		q = &transac_queue[pos];
		list_push(q, n);
		// <==== balancly putting to transaction queue
	}
	// cleanup and exit
	rte_atomic32_dec(&prog_ctl.thr_num);
	pthread_exit(NULL);
	return NULL;
}


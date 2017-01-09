#include "list.h"
#include "global.h"
#include "pkt_pool.h"
#include "packet_decode.h"
#include "transaction_thread.h"
#include "transaction_store.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>

list_t *transac_queue = NULL;

void *transaction_thread(void *arg) {
	node_t *n;
	list_t tmpq;
	pkt_buffer *pktbf;
	packet_t *pkt = NULL;
	size_t sz = sizeof(packet_t);
	int idx = (int)(unsigned long)arg;
	list_t *q = &transac_queue[idx];
	struct timespec req = {0, 10};
        rte_atomic32_inc(&prog_ctl.thr_num);
	// to-do: set cpu affinity
	pthread_detach(pthread_self());

	struct timeval old, cur;
	int16_t delay;
	uint64_t pkt_cnt = 0;
	gettimeofday(&old, NULL);

	list_create(&tmpq);
	pkt = calloc(1, sizeof(packet_t));
	while(rte_atomic32_read(&prog_ctl.run)) {
		list_split(q, &tmpq);
		if(tmpq.head == NULL) { // tmpq is empty
			nanosleep(&req, NULL);
			continue;
		}
		n = tmpq.head;
		while(n != NULL) {
			memset(pkt, 0, sz);
			pktbf = (pkt_buffer*)n->data;
			packet_decode(pkt, (uint8_t *)pktbf->pkt, pktbf->len);
			packet_store_output(pkt);
			// counter increase after one packet has been processed
			++pkt_cnt;
			// skip to the next packet
			n = n->next;
		}
		// return nodes back to packet pool
		list_merge(&tmpq, &pack_pool.pkt_list);
		// statistics of packet processing speed
		gettimeofday(&cur, NULL);
		delay = cur.tv_sec - old.tv_sec;
		if(delay >= 60) {
			printf("[DEBUG] transaction thread[%d] processing speed %.2f pps\n",idx,pkt_cnt/(float)delay);
			pkt_cnt = 0;
		}
	}

	rte_atomic32_dec(&prog_ctl.thr_num);
	free(pkt);
	pthread_exit(NULL);
	return NULL;
}


#include "list.h"
#include "global.h"
#include "pkt_pool.h"
#include "packet_decode.h"
#include "transaction_thread.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>

list_t *transac_queue = NULL;

void *transaction_thread(void *arg) {
	// to-do: set cpu affinity
	pthread_detach(pthread_self());
        rte_atomic32_inc(&prog_ctl.thr_num);
	int idx = (int)(unsigned long)arg;
	list_t *q = &transac_queue[idx];
	node_t *n;
	pkt_buffer *pktbf;
	struct timespec req = {0, 10};
	packet_t *pack = NULL;

	pack = calloc(1, sizeof(packet_t));
	while(rte_atomic32_read(&prog_ctl.run)) {
		while((n = list_pop(q)) == NULL) {
			nanosleep(&req, NULL);
		}
		pktbf = (pkt_buffer*)n->data;
		packet_decode(pack, pktbf->pkt, pktbf->len);
		pkt_pool_free(&pack_pool, n);
	}

        rte_atomic32_dec(&prog_ctl.thr_num);
	free(pack);
	pthread_exit(NULL);
	return NULL;
}


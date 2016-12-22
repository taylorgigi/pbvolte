#include "list.h"
#include "global.h"
#include "transaction_thread.h"
#include <stdlib.h>
#include <pthread.h>

list_t *transac_queue = NULL;

void *transaction_thread(void *arg) {
	// to-do: set cpu affinity
	pthread_detach(pthread_self());
        rte_atomic32_inc(&prog_ctl.thr_num);
	int idx = (int)(unsigned long)arg;
	list_t *q = &transac_queue[idx];

        rte_atomic32_dec(&prog_ctl.thr_num);
	pthread_exit(NULL);
	return NULL;
}


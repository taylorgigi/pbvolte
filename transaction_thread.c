#include "list.h"
#include "transaction_thread.h"
#include <stdlib.h>
#include <pthread.h>

void *transaction_thread(void *arg) {
	// to-do: set cpu affinity
	pthread_detach(pthread_self());
	int idx = (int)(unsigned long)arg;
	list_t *que = transac_queue[idx];

	pthread_exit(NULL);
	return NULL;
}


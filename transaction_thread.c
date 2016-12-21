#include "transaction_thread.h"
#include <stdlib.h>
#include <pthread.h>

void *transaction_thread(void *arg) {
	// to-do: bind cpu
	pthread_detach(pthread_self());

	int idx = (int)(unsigned long)arg;

	pthread_exit(NULL);
	return NULL;
}


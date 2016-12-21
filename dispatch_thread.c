#include "dispatch_thread.h"
#include <stdlib.h>
#include <pthread.h>

void *dispatch_thread(void *arg) {
	// to-do: bind cpu
	pthread_detach(pthread_self());

	pthread_exit(NULL);
	return NULL;
}


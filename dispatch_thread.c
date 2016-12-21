#include "global.h"
#include "dispatch_thread.h"
#include <stdlib.h>
#include <pthread.h>

// dispatch packet to transaction thread according to src/dst ip pair
void *dispatch_thread(void *arg) {
	// to-do: set cpu affinity
	pthread_detach(pthread_self());
	rte_atomic32_inc(&prog_ctl.thr_num);
	// keep running?
	while(rte_atomic32_read(&prog_ctl.run)) {
	}
	// cleanup and exit
	rte_atomic32_dec(&prog_ctl.thr_num);
	pthread_exit(NULL);
	return NULL;
}


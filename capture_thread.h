#ifndef CAPTURE_THREAD_H
#define CAPTURE_THREAD_H

#include <stdint.h>
#include <sys/time.h>

#define SNAPSHOT_LEN                 2000

#define PFRING_DIRECTION_RXTX        0
#define PFRING_DIRECTION_RX          1
#define PFRING_DIRECTION_TX          2

#define NOWAIT                       0
#define WAIT                         1

typedef struct cap_statis {
	uint32_t pkt_old;
	uint32_t pkt_cur;
	uint32_t byte_old;
	uint32_t byte_cur;
	struct timeval tv_old;
	struct timeval tv_cur;
} cap_statis;

extern list_t dispatch_queue;

void *live_thread_pfring(void *arg);
void *live_thread_pcap(void *arg);

void *offline_thread(void *arg);

#endif


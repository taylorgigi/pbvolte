#ifndef DISPATCH_THREAD_H
#define DISPATCH_THREAD_H

#include <stdint.h>

// dispatch key
typedef struct dis_key {
	uint64_t ip_sum;
} dis_key;

void *dispatch_thread(void *arg);

#endif


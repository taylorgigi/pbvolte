#ifndef GLOBAL_H
#define GLOBAL_H

#include "rte_atomic.h"

typedef struct prog_state {
	rte_atomic32_t run;
	rte_atomic32_t thr_num;
} prog_state;

extern prog_state prog_ctl;
extern int sigfd;
extern int efd;
extern struct epoll_event events;

#endif


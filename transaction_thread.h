#ifndef TRANSACTION_THREAD_H
#define TRANSACTION_THREAD_H

#include "list.h"

extern list_t *transac_queue;

void *transaction_thread(void *arg);

#endif


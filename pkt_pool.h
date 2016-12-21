#ifndef PKT_POOL_H
#define PKT_POOL_H

#include "list.h"
#include <stdint.h>
#include <sys/time.h>

// limit of packet length for ethernet(1518)
#define PKT_MAX        1600

typedef struct pkt_buffer {
	struct timeval ts;                  // timestamp of packet(sec, usec)
	uint16_t len;                       // length of packet in byte
	char pkt[PKT_MAX];                  // packet data buffer
} pkt_buffer;

typedef struct pkt_pool {
	rte_atomic32_t size;                // number of packet can contain
	rte_atomic32_t used;                // number of used buffers
	char *node_mem;                     // memory buffer allocated for node_t
	char *pkt_mem;                      // memory buffer allocated for pkt_buffer
	list_t pkt_list;                    // packet list
} pkt_pool;

extern pkt_pool pack_pool;

int pkt_pool_create(pkt_pool *pool, uint16_t nb_pkt);

void pkt_pool_destroy();

// get a node_t buffer from pool(node_t removed from pool)
// return NULL on failed (pool memory used up)
node_t* pkt_pool_alloc(pkt_pool *pool);

// give node_t back to pool
void pkt_pool_free(pkt_pool *pool, node_t *n);

#endif


#include "pkt_pool.h"
#include <string.h>
#include <stdlib.h>

pkt_pool pack_pool;

// create packet pool
int pkt_pool_create(pkt_pool *pool, uint16_t nb_pkt) {
	memset(pool, 0, sizeof(pkt_pool));
	rte_atomic32_set(&pool->size, nb_pkt);
	// initialize list
	list_create(&pool->pkt_list);
	// allocate memory for node_t
	pool->node_mem = calloc(nb_pkt, sizeof(node_t));
	if(pool->node_mem == NULL)
		return -1;
	// allocate memory for pkt_buffer
	pool->pkt_mem = calloc(nb_pkt, sizeof(pkt_buffer));
	if(pool->pkt_mem == NULL) {
		if(pool->node_mem) {
			free(pool->node_mem);
			pool->node_mem = NULL;
		}
		return -1;
	}
	// insert to list
	int i = 0;
	node_t *n = NULL;
	pkt_buffer *pkt = NULL;
	for(i=0; i<nb_pkt; ++i) {
		n = (node_t*)(pool->node_mem + i * sizeof(node_t));
		pkt = (pkt_buffer*)(pool->pkt_mem + i * sizeof(pkt_buffer));
		n->data = pkt;
		list_push(&pool->pkt_list, n);
	}
	return 0;
}

// destroy packet pool
void pkt_pool_destroy(pkt_pool *pool) {
	if(pool->pkt_mem)
		free(pool->pkt_mem), pool->pkt_mem = NULL;
	if(pool->node_mem)
		free(pool->node_mem), pool->node_mem = NULL;
}

node_t *pkt_pool_alloc(pkt_pool *pool) {
	node_t *n = list_pop(&pool->pkt_list);
	if(n)
		rte_atomic32_inc(&pool->used);
	return n;
}

void pkt_pool_free(pkt_pool *pool, node_t *n) {
	list_push(&pool->pkt_list, n);
	rte_atomic32_dec(&pool->used);
}


#ifdef _cplusplus
extern "C" {
#endif

#include "list.h"

// init list to empty list
void list_create(list_t *list) {
	rte_atomic32_init(&list->num);
	list->head = NULL;
	list->tail = NULL;
	pthread_spin_init(&list->lock, PTHREAD_PROCESS_PRIVATE);
}

// pop all node
void list_destroy(list_t *list) {
	while(list_pop(list) != NULL) {
		// do nothing
	}
	pthread_spin_destroy(&list->lock);
}

// insert a node to tail of list
void list_push(list_t *list, node_t *n) {
	pthread_spin_lock(&list->lock);
	if(list->head == NULL) {
		list->tail = list->head = n;
		n->prev = NULL;
		n->next = NULL;
	}
	else {
		list->tail->next = n;
		n->prev = list->tail;
		n->next = NULL;
		list->tail = n;
	}
	rte_atomic32_inc(&list->num);
	pthread_spin_unlock(&list->lock);
}

// remove a node from head of list
node_t *list_pop(list_t *list) {
	node_t dumy;
	dumy.next = NULL;
	dumy.prev = NULL;
	dumy.data = NULL;
	node_t *p = NULL;
	pthread_spin_lock(&list->lock);
	dumy.next = list->head;
	if(dumy.next != NULL) {
		p = dumy.next;
		dumy.next = list->head->next;
		p->next = NULL;
		list->head = dumy.next;
		if(list->head != NULL)
			list->head->prev = NULL;
		else
			list->tail = list->head;
		rte_atomic32_dec(&list->num);
	}
	pthread_spin_unlock(&list->lock);
	return p;
}

// move all elements of list src to dst(must be empty), so list src empty
void list_split(list_t *src, list_t *dst) {
	pthread_spin_lock(&src->lock);
	// src empty, return
	if(src->head == NULL) {
		pthread_spin_lock(&src->lock);
		return;
	}
	if(dst->head == NULL) {
		dst->head = src->head;
		dst->tail = src->tail;
	}
	else {
		dst->tail->next = src->head;
		src->head->prev = dst->tail;
		dst->tail = src->tail;
	}
	rte_atomic32_add(&dst->num,rte_atomic32_read(&src->num));
	src->tail = src->head = NULL;
	rte_atomic32_set(&src->num, 0);
	pthread_spin_unlock(&src->lock);
}

// move all elements of list src to dst, then src empty
void list_merge(list_t *src, list_t *dst) {
	if(src->head == NULL)
		return;
	pthread_spin_lock(&dst->lock);
	if(dst->head == NULL) {
		dst->head = src->head;
		dst->tail = src->tail;
	}
	else {
		dst->tail->next = src->head;
		src->head->prev = dst->tail;
		dst->tail = src->tail;
	}
	rte_atomic32_add(&dst->num,rte_atomic32_read(&src->num));
	src->tail = src->head = NULL;
	rte_atomic32_set(&src->num, 0);
	pthread_spin_unlock(&dst->lock);
}

#ifdef _cplusplus
}
#endif


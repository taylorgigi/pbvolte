#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>
#include <pthread.h>

typedef struct hash_bucket_t {
	uint32_t hash_val;               // hash value
	uint32_t verify_hash_val1;       // verify hash value 1
	uint32_t verify_hash_val2;       // verify hash value 2
	pthread_spinlock_t lock;
	void *data;                      // this bucket is empty(not occupied) when data is NULL
} hash_bucket;

typedef struct hash_table_t {
	rte_atomic32_t size;             // total number of buckets when created
	rte_atomic32_t used;             // used(occupied) number of buckets
	hash_bucket *buckets;

	uint32_t (*hash)(void *key, int len);
	uint32_t (*verify_hash1)(void *key, int len);
	uint32_t (*verify_hash2)(void *key, int len);
} hash_table;

void hash_table_create(hash_table *ht, uint32_t size);
void hash_table_destroy(hash_table *ht);

// find bucket by key.
// return a locked-bucket if exist, or NULL if not exist
hash_bucket *hash_table_find(hash_table *ht, void *key, int len);

// insert a bucket by key
// return -1 if hash table is full
// return pos if insert successfully or key exists
int32_t hash_table_insert(hash_table *ht, void *key, int len);

#endif


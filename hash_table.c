#include "hash.h"
#include "hash_table.h"
#include <stdlib.h>
#include <string.h>

int hash_table_create(hash_table *ht, uint32_t size) {
	uint32_t i;
	memset(ht, 0, sizeof(hash_table));
	rte_atomic32_set(&ht->size, size);
	rte_atomic32_init(&ht->used);
	ht->hash = &hash;
	ht->verify_hash1 = &verify_hash1;
	ht->verify_hash2 = &verify_hash2;
	ht->buckets = calloc(size, sizeof(hash_bucket));
	if(ht->buckets == NULL)
		return -1;
	for(i=0; i<size; ++i) {
		pthread_spin_init(&ht->buckets[i].lock, PTHREAD_PROCESS_PRIVATE);
	}
	return 0;
}

// find bucket by key.
// return a locked-bucket if exist, or NULL if not exist
hash_bucket *hash_table_find(hash_table *ht, void *key, int len) {
	uint32_t hash_val, verify_val1, verify_val2;
	hash_val = ht->hash(key, len);
	verify_val1 = ht->verify_hash1(key, len);
	verify_val2 = ht->verify_hash2(key, len);
	uint32_t pos = hash_val % rte_atomic32_read(&ht->size);
	uint32_t orig = pos;
	hash_bucket *bucket = NULL;
	do {
		bucket = &ht->buckets[pos];
		pthread_spin_lock(&bucket->lock);
		if(bucket->data != NULL) {
			if(hash_val == bucket->hash_val && verify_val1 == bucket->verify_hash_val1 && verify_val2 == bucket->verify_hash_val2) {
				return bucket;
			}
		}
		else {
			pthread_spin_unlock(&bucket->lock);
			return NULL;
		}
		pthread_spin_unlock(&bucket->lock);
		pos = (pos + 1) % rte_atomic32_read(&ht->size);
	} while(pos != orig);
	return NULL;
}

// insert a bucket by key
// return NULL if hash table is full
// return a locked-bucket if insert successfully or key exists
hash_bucket *hash_table_insert(hash_table *ht, void *key, int len, void *data) {
	uint32_t hash_val, verify_val1, verify_val2;
	hash_val = ht->hash(key, len);
	verify_val1 = ht->verify_hash1(key, len);
	verify_val2 = ht->verify_hash2(key, len);
	uint32_t pos = hash_val % rte_atomic32_read(&ht->size);
	uint32_t orig = pos;
	hash_bucket *bucket = NULL;
	do {
		bucket = &ht->buckets[pos];
		pthread_spin_lock(&bucket->lock);
		if(bucket->data == NULL) {
			bucket->hash_val = hash_val;
			bucket->verify_hash_val1 = verify_val1;
			bucket->verify_hash_val2 = verify_val2;
			bucket->data = data;
			return bucket;
		}
		else if(hash_val == bucket->hash_val && verify_val1 == bucket->verify_hash_val1 && verify_val2 == bucket->verify_hash_val2) {
			return bucket;
		}
		pthread_spin_unlock(&bucket->lock);
		pos = (pos + 1) % rte_atomic32_read(&ht->size);
	} while(pos != orig);
	return NULL;
}


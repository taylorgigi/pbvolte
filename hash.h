#ifndef HASH_INCLUDE_H
#define HASH_INCLUDE_H

#include <stdint.h>

uint32_t hash(void *key, uint32_t len);
uint32_t verify_hash1(void *key, uint32_t len);
uint32_t verify_hash2(void *key, uint32_t len);

#endif


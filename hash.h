#ifndef HASH_INCLUDE_H
#define HASH_INCLUDE_H

#include <stdint.h>

uint32_t hash(const char *key, uint32_t len);
uint32_t verify_hash1(const char *key, uint32_t len);
uint32_t verify_hash2(const char *key, uint32_t len);

#endif

